# PandOS
µPandOS is an educational microkernel operating system

## Installation and usage
1. To use this software you need to install <i>umps3</i>  with your package manager, for example with apt:
    ```sh
    sudo apt install umps3
    ```

2. Then, you need to clone that repository:
    ```sh
    git clone https://github.com/aNdReA9111/PandOS.git
    ```

3. open the folder of the project and compile with makefile:
    ```sh
    cd PandOS && make
    ```

4. then run <i>umps3</i>  with the following command amd create a new machine with the path of the folder of PandOS:
    ```sh
    umps3
    ```

5. Use the command palette of <i>umps3</i> to run.

## Structure
- `headers`: contains the costant and some usefull macro;
- `phase1`: contains the definition of functions about pcb lists, pcbs trees, messages lists;
- `phase2`: contains the Scheduler, SSI process, Exception and Interrupt handlers and the Nucleus initialization;
- `phase3`: contains the support level (User process, swap mutex process and device process initialization, Exception handling at support level, System Service Thread and virtual memory).

For more information about project and/or authors (`AUTHOR`), see the report: `Relazione.pdf`

## Makefile

- To compile project use:
    ```sh
    make
    ```  
- To delete  `.o`,  `.umps` and  `kernel` use:
    ```sh
    make clean
    ```  
- To compile testers use:
    ```sh
    cd testers
    make
    ```  
- To delete tersters use:
    ```sh
    cd testers
    make clean
    ``` 
  
## License
This project is released under the Creative Commons License. See the `LICENSE` file for more details.
