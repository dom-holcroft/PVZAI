import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
import os

NUM_ROWS = 5
NUM_COLS = 9
NUM_SEEDS = 10
NUM_CHANNELS = 28


class Multi_QNet(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(
            in_channels=NUM_CHANNELS,
            out_channels=16,
            kernel_size=(1, 10),
            padding="same",
        )
        self.conv2 = nn.Conv2d(
            in_channels=16, out_channels=32, kernel_size=(3, 3), padding="same"
        )

        self.linear1 = nn.Linear(21, 50)

        self.flatten = nn.Flatten()

        self.fc1 = nn.Linear(64 * 5 * 10, 100)
        self.fc2 = nn.Linear(150, 150)

        self.col_head = nn.Linear(150, NUM_COLS)

        # Head for predicting y coordinate
        self.row_head = nn.Linear(150, NUM_ROWS)

        # Head for seed selection
        self.seed_head = nn.Linear(150, NUM_SEEDS)

    def forward(self, state):
        grid, linear_input = state
        grid = torch.tensor(grid, dtype=torch.float)
        linear_input = torch.tensor(linear_input, dtype=torch.float)

        grid = self.conv1(grid)
        grid = self.conv2(grid)

        grid = self.flatten(grid)

        x_fc1 = self.fc1(grid)

        x_linear1 = self.linear1(linear_input)

        combined = torch.cat(
            (x_fc1, x_linear1), dim=1
        ) 

        x_fc2 = self.fc2(combined)

        # Additional heads
        col_out = self.col_head(x_fc2)
        row_out = self.row_head(x_fc2)
        seed_out = self.seed_head(x_fc2)

        col_out = F.softmax(col_out, dim=1)
        row_out = F.softmax(row_out, dim=1)
        seed_out = F.softmax(seed_out, dim=1)

        return col_out, row_out, seed_out

    def save(self, file_name="model.pth"):
        model_folder_path = "./model"
        if not os.path.exists(model_folder_path):
            os.makedirs(model_folder_path)

        file_name = os.path.join(model_folder_path, file_name)
        torch.save(self.state_dict(), file_name)


class QTrainer:
    def __init__(self, model, lr, gamma):
        self.lr = lr
        self.gamma = gamma
        self.model = model
        self.optimiser = optim.Adam(model.parameters(), lr=self.lr)
        self.criterion = nn.MSELoss

    def train_step(self, state, action, reward, state_new, game_over):
        grid, linear_input = state
        next_grid, next_linear_input = state_new

        grid = torch.tensor(grid, dtype=torch.float32)
        linear_input = torch.tensor(linear_input, dtype=torch.float32)
        next_grid = torch.tensor(next_grid, dtype=torch.float32)
        next_linear_input = torch.tensor(next_linear_input, dtype=torch.float32)
        action = torch.tensor(action, dtype=torch.long)
        reward = torch.tensor(reward, dtype=torch.float32)

        # Add batch dimension if needed
        if len(grid.shape) == 3:
            grid = grid.unsqueeze(0)
            linear_input = linear_input.unsqueeze(0)
            next_grid = next_grid.unsqueeze(0)
            next_linear_input = next_linear_input.unsqueeze(0)
            action = action.unsqueeze(0)
            reward = reward.unsqueeze(0)
            game_over = torch.tensor([game_over])

        # Predict Q-values for the current state
        pred_col, pred_row, pred_seed = self.model((grid, linear_input))

        # Clone predictions to create the target
        target_col = pred_col.clone()
        target_row = pred_row.clone()
        target_seed = pred_seed.clone()

        # Predict Q-values for the next state
        next_pred_col, next_pred_row, next_pred_seed = self.model((next_grid, next_linear_input))

        # Determine the Q-value target
        for idx in range(len(game_over)):
            Q_new = reward[idx]
            if not game_over[idx]:
                Q_new = reward[idx] + self.gamma * torch.max(
                    torch.cat((
                        next_pred_col[idx].flatten(),
                        next_pred_row[idx].flatten(),
                        next_pred_seed[idx].flatten()
                    ))
                )

            # Update the targets with the new Q-value
            target_col[idx][action[idx][0]] = Q_new
            target_row[idx][action[idx][1]] = Q_new
            target_seed[idx][action[idx][2]] = Q_new

        # Zero the gradient
        self.optimiser.zero_grad()

        # Calculate loss
        loss_col = self.criterion(pred_col, target_col)
        loss_row = self.criterion(pred_row, target_row)
        loss_seed = self.criterion(pred_seed, target_seed)

        # Total loss
        loss = loss_col + loss_row + loss_seed

        # Backpropagation
        loss.backward()

        # Update weights
        self.optimiser.step()
