import re

def preprocess_line(line:str):
    # Remove leading and trailing whitespaces
    line = line.strip()
    # Replace multiple whitespaces with a single whitespace
    line = re.sub(r"\s+", " ", line)
    return line

def process_line(line: str):
    # Split the line by whitespaces
    line = preprocess_line(line)
    parts = line.split(" ")
    all_time = re.findall(r"\d+\.\d+e[+-]?\d+", line)
    if len(all_time) == 0:
        return None
    print(all_time)
    kernel_name = parts[0]
    try:
        t = float(parts[1])
    except:
        kernel_name += " " + parts[1]
    total_time = all_time[0]
    return {"kernel": kernel_name, "total_time": total_time}
    
def extract_time(input_text: str):
    lines = input_text.split("\n")
    start_line = "******* Timers for MPE *******"

    is_in_time_section = False
    time_info = []
    for line in lines:
        if not is_in_time_section:
          if line == start_line:
              is_in_time_section = True
              continue
        else:
          if line.startswith("Timer label"):
              continue
          if line.startswith("_"):
              continue
          if line == "":
              continue
          kernel_time = process_line(line)
          if kernel_time:
            time_info.append(kernel_time)
    return time_info
        
# Example input
input_text = """
Job <6261713> has been submitted to queue <q_sw_expr>
waiting for dispatch ...
Using YAKL Timers
Using memory pool. Initial size: 1.83105GB ;  Grow size: 1.83105GB.
Running on SW 1CG mode 
=========================================
stencil-2d
=========================================
=========================================
stencil-3d
=========================================
=========================================
stencil-1d
=========================================
=========================================
heat-2d
=========================================
=========================================
jacobi-2d
=========================================
=========================================
fdtd-2d
=========================================
-----------------------------------------
YAKL parallel_for
-----------------------------------------
-----------------------------------------
YAKL parallel_for simd
-----------------------------------------
Pool Memory High Water Mark:       2172748032
Pool Memory High Water Efficiency: 0.552558
******* Timers for MPE *******
________________________________________________________________________________________________________
Timer label                                       # calls     Total time     Min time       Max time       
________________________________________________________________________________________________________
stencil-2d-5 serial                               20          1.479231e+01   7.363783e-01   7.434717e-01   
stencil-2d-5 kernel                               20          2.470552e+00   1.230679e-01   1.241661e-01   
stencil-2d-5 simd                                 20          1.308516e+00   6.534357e-02   6.551266e-02   
stencil-2d-9 serial                               20          1.358422e+01   6.766549e-01   6.822950e-01   
stencil-2d-9                                      20          3.462311e+00   1.719099e-01   1.741691e-01   
stencil-2d-9 simd                                 20          1.946998e+00   9.716608e-02   9.779557e-02   
stencil-3d-7 serial                               20          2.230922e+01   1.108813e+00   1.120737e+00   
stencil-3d-7                                      20          2.281980e-03   1.118490e-04   1.181990e-04   
stencil-3d-7 simd                                 20          2.534028e-03   1.238490e-04   1.339980e-04   
stencil-3d-27 serial                              20          1.308569e+02   6.516392e+00   6.565120e+00   
stencil-3d-27                                     20          2.429530e-03   1.184490e-04   1.278490e-04   
stencil-3d-27 simd                                20          2.663373e-03   1.295980e-04   1.571990e-04   
stencil-1d-3 serial                               20          9.203815e+00   4.584898e-01   4.617248e-01   
stencil-1d-3 kernel                               20          5.319950e-04   1.745000e-05   3.760000e-05   
stencil-1d-3 simd                                 20          5.868440e-04   1.889900e-05   4.649900e-05   
heat-2d serial                                    20          1.634112e+01   8.144436e-01   8.199448e-01   
heat-2d kernel                                    20          2.167370e+00   1.079253e-01   1.086554e-01   
heat-2d kernel simd                               20          1.831421e+00   9.146873e-02   9.166208e-02   
jacobi-2d serial                                  20          1.731407e+01   8.629447e-01   8.696573e-01   
jacobi-2d kernel                                  20          2.780819e+00   1.386271e-01   1.395660e-01   
jacobi-2d kernel simd                             20          1.062950e+00   5.305608e-02   5.348947e-02   
fdtd-2d serial                                    128         1.685337e+02   1.312213e+00   1.325900e+00   
fdtd-2d kernel                                    128         3.217450e+01   2.504710e-01   2.523270e-01   
fdtd-2d kernel simd                               128         2.361788e+01   1.829415e-01   1.865286e-01   
________________________________________________________________________________________________________
The ~ character beginning a timer label indicates it has multiple parent timers.
Thus, those timers will likely not accumulate like you expect them to.


dispatching ...
Job 6261713 has been finished.
"""

# Extract total times
total_times = extract_time(input_text)

# Print the extracted total times
for time in total_times:
    print(time)
