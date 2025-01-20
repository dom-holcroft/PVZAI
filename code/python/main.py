from pvzinterface import Process
import numpy as np
import random
import time




def main():
    previous_time = 0
    pvz = Process("popcapgame1.exe")

    pvz.setup_code_injection()
    while(True):

        if(pvz.is_game_over()):
            previous_time = 0  
            print("game over") 
            pvz.restart_game() 
            while(not pvz.is_game_running()):
                #time.sleep(0.001)
                pass
        print("1")
        gameValues = pvz.get_game_values()
        print("2")
        if(gameValues): 
            a,b = gameValues
            print(a[10])
            pvz.place_plant(random.randrange(0,9),random.randrange(0,5),random.randrange(0,10))
            previous_time, reward = pvz.play_step(previous_time)

            print(reward)
        print("3")

if __name__ == "__main__":
    main()