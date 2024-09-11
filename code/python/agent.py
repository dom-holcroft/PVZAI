import torch
import random
import numpy as np
from pvzinterface import Process
from collections import deque
from model import Multi_QNet, QTrainer

MAX_MEMORY = 1_000_000
BATCH_SIZE = 1000
LR = 0.001


class Agent:
    def __init__(self, PVZ: Process):
        self.PVZ = PVZ
        self.n_games = 0
        self.epsilon = 0
        self.gamma = 0.9
        self.memory = deque(maxlen=MAX_MEMORY)
        self.model = Multi_QNet()
        self.trainer = QTrainer(self.model, lr=LR, gamma=self.gamma)

    def remember(self, state, action, reward, next_state, game_over):
        self.memory.append((state, action, reward, next_state, game_over))

    def train_long_memory(self):
        if len(self.memory) > BATCH_SIZE:
            mini_sample = random.sample(self.memory, BATCH_SIZE)
        else:
            mini_sample = self.memory

        states, actions, rewards, next_states, game_overs = zip(*mini_sample)
        self.trainer.train_step(states, actions, rewards, next_states, game_overs)
        for state, action, reward, next_state, game_over in mini_sample:
            self.trainer.train_step(state, action, reward, next_state, game_over)

    def train_short_memory(self, state, action, reward, next_state, game_over):
        self.trainer.train_step(state, action, reward, next_state, game_over)

    def get_action(self, state):
        self.epsilon = 80 - self.n_games
        final_move_row = np.zeros(5)
        final_move_col = np.zeros(10)
        final_move_seed = np.zeros(10)
        if random.randint(0,200) < self.epsilon:
            final_move_row[random.randint(5)] = 1
            final_move_col[random.randint(10)] = 1
            final_move_seed[random.randint(10)] = 1
        else:
            colPrediction, rowPrediction, seedPrediction = self.model(state)
            final_move_col[torch.argmax(colPrediction).item()]
            final_move_row[torch.argmax(rowPrediction).item()]
            final_move_seed[torch.argmax(seedPrediction).item()]

        return final_move_col, final_move_row, final_move_seed


def setup():
    pvz = Process("popcapgame1.exe")
    pvz.setup_code_injection()
    return pvz


def train(PVZ: Process):
    plot_scores = []
    plot_mean_scores = []
    total_score = 0
    record = 0
    agent = Agent(PVZ)
    previous_time = 0
    while True:
        # get old state
        last_state = PVZ.get_game_values()
        final_move = agent.get_action(last_state)
        previous_time, reward = PVZ.play_step(previous_time)
        game_over = PVZ.is_game_over()
        new_state = PVZ.get_game_values()

        agent.train_short_memory(last_state, final_move, reward, new_state, game_over)

        agent.remember(last_state, final_move, reward, new_state, game_over)

        if PVZ.is_game_over():
            agent.n_games += 1
            agent.train_long_memory()

            previous_time = 0
            print("game over")
            PVZ.restart_game()
            while not PVZ.is_game_running():
                pass


if __name__ == "__main__":
    PVZ = setup()
    train(PVZ)
