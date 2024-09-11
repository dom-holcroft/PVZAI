from pymem import Pymem
import machinecode
import pvz as pvz
from pvz import get_clock_time, read_environment, game_running
from util import get_action_interval
import sys


def attatchToProcess() -> Pymem:
    process = Pymem("popcapgame1.exe")
    return process


def wait_till_next_action(process: Pymem, current_time, setupVariables):
    delay = get_action_interval()
    while get_clock_time(process) - current_time < delay:
        if(process.read_int(setupVariables.restart_flag_address)):
            break

    current_time = current_time + delay 
    return current_time


def game_loop(process, setupVariables):
    current_time = 0
    
    while(game_running(process)):
        #read_environment(process, setupVariables.seeds)
            #if pvz.chseck_game_running(process):
        pvz.plant_plant(process, 1, 1, 3, setupVariables.plantPlantAddresses)
                #count += 1

        
        current_time = wait_till_next_action(process, current_time, setupVariables)


def main():
    process = attatchToProcess()
    setupVariables = machinecode.setup_code_injection(process)
    #current_time = get_clock_time(process)
    while True:
        game_loop(process, setupVariables)  
        print(sys.getrefcount)
        restart_flag_value = process.read_int(setupVariables.restart_flag_address)
        if restart_flag_value == 1:
            process.write_int(setupVariables.restart_flag_address, 0)
            process.write_int(setupVariables.plantPlantAddresses.seed_slot, -1)
            print("restart")



    while True:
       pass 

if __name__ == "__main__":
    main()
