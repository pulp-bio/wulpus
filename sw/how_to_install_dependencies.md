# How to install Python requirements?
1. Install Anaconda package manager<br>
   https://docs.conda.io/en/latest/miniconda.html
2. Find `requirements.yaml` file in `dev_pc` folder
3. Open terminal (Windows: Anaconda Prompt) in `dev_pc` folder.
4. Execute the following command to create environment:
```
    conda env create -f requirements.yml
```
5. In a new terminal launch 
```
    conda activate wulpus_env
```
6. and then run the command below:<br>
   (or launch it from Start Menu on Windows:<br>
   *Start -> Anaconda3 -> Jupyter Notebook (wupus_env)*)
```
    jupyter notebook
```
   

7. The command above opens a webpage. Navigate to `dev_pc` folder and click on `wulpus_gui.ipynb`. 
   Then, follow the instructions in the Notebook.
