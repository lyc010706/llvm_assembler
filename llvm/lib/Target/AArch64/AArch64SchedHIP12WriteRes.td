let Latency = 1, NumMicroOps = 1 in
def HIP12Write_1c_1B : SchedWriteRes<[HIP12UnitB]> {
}

let Latency = 1, NumMicroOps = 2 in
def HIP12Write_1c_1B_1ALU14 : SchedWriteRes<[HIP12UnitB, HIP12UnitALU14]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP12Write_1c_1ALU : SchedWriteRes<[HIP12UnitALU]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP12Write_1c_1ALU1425 : SchedWriteRes<[HIP12UnitALU1425]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP12Write_2c_1ALU25 : SchedWriteRes<[HIP12UnitALU25]> {
}

let Latency = 1, NumMicroOps = 2 in
def HIP12Write_1c_1ALU14_1ALU25 : SchedWriteRes<[HIP12UnitALU14, HIP12UnitALU25]> {
}

let Latency = 1, NumMicroOps = 1 in
def HIP12Write_1c_1ALU14 : SchedWriteRes<[HIP12UnitALU14]> {
}

let Latency = 6, NumMicroOps = 2 in
def HIP12Write_6c_2ALU25 : SchedWriteRes<[HIP12UnitALU25]> {
  let ResourceCycles = [2];
}

let Latency = 8, NumMicroOps = 2 in
def HIP12Write_8c_2ALU25 : SchedWriteRes<[HIP12UnitALU25]> {
  let ResourceCycles = [2];
}

let Latency = 3, NumMicroOps = 1 in
def HIP12Write_3c_1ALU25 : SchedWriteRes<[HIP12UnitALU25]> {
}

let Latency = 3, NumMicroOps = 4 in
def HIP12Write_3c_1ALU25_3ALU : SchedWriteRes<[HIP12UnitALU25, HIP12UnitALU]> {
  let ResourceCycles = [1, 3];
}

let Latency = 4, NumMicroOps = 4 in
def HIP12Write_4c_1ALU25_3ALU : SchedWriteRes<[HIP12UnitALU25, HIP12UnitALU]> {
  let ResourceCycles = [1, 3];
}

let Latency = 4, NumMicroOps = 2 in
def HIP12Write_4c_2ALU25 : SchedWriteRes<[HIP12UnitALU25]> {
  let ResourceCycles = [2];
}

let Latency = 5, NumMicroOps = 4 in
def HIP12Write_5c_2ALU25_2B : SchedWriteRes<[HIP12UnitALU25, HIP12UnitB]> {
  let ResourceCycles = [2, 2];
}

let Latency = 8, NumMicroOps = 5 in
def HIP12Write_8c_3LD_2ALU25 : SchedWriteRes<[HIP12UnitLD, HIP12UnitALU25]> {
  let ResourceCycles = [3, 2];
}

let Latency = 2, NumMicroOps = 2 in
def HIP12Write_2c_2ALU25 : SchedWriteRes<[HIP12UnitALU25]> {
  let ResourceCycles = [2];
}

let Latency = 4, NumMicroOps = 1 in
def HIP12Write_4c_1LD : SchedWriteRes<[HIP12UnitLD]> {
}

let Latency = 4, NumMicroOps = 3 in
def HIP12Write_4c_1LD_2ALU : SchedWriteRes<[HIP12UnitLD, HIP12UnitALU]> {
  let ResourceCycles = [1, 2];
}

let Latency = 1, NumMicroOps = 2 in
def HIP12Write_1c_1ST_1STD : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD]> {
}

let Latency = 1, NumMicroOps = 5 in
def HIP12Write_1c_1ST_1STD_3ALU : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD, HIP12UnitALU]> {
  let ResourceCycles = [1, 1, 3];
}

let Latency = 1, NumMicroOps = 1 in
def HIP12Write_1c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP12Write_2c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 2, NumMicroOps = 3 in
def HIP12Write_2c_1V02_2ALU0134 : SchedWriteRes<[HIP12UnitV02, HIP12UnitALU0134]> {
  let ResourceCycles = [1, 2];
}

let Latency = 4, NumMicroOps = 3 in
def HIP12Write_4c_1ALU14_2V : SchedWriteRes<[HIP12UnitALU14, HIP12UnitV]> {
  let ResourceCycles = [1, 2];
}

let Latency = 6, NumMicroOps = 3 in
def HIP12Write_6c_1ALU14_2V : SchedWriteRes<[HIP12UnitALU14, HIP12UnitV]> {
  let ResourceCycles = [1, 2];
}

let Latency = 4, NumMicroOps = 1 in
def HIP12Write_4c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 6, NumMicroOps = 1 in
def HIP12Write_6c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 8, NumMicroOps = 1 in
def HIP12Write_8c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 3, NumMicroOps = 1 in
def HIP12Write_3c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 7, NumMicroOps = 3 in
def HIP12Write_7c_1ALU14_2V : SchedWriteRes<[HIP12UnitALU14, HIP12UnitV]> {
  let ResourceCycles = [1, 2];
}

let Latency = 5, NumMicroOps = 1 in
def HIP12Write_5c_1V : SchedWriteRes<[HIP12UnitV]> {
}

let Latency = 5, NumMicroOps = 1 in
def HIP12Write_5c_1V02 : SchedWriteRes<[HIP12UnitV02]> {
}

let Latency = 4, NumMicroOps = 1 in
def HIP12Write_4c_1ALU14 : SchedWriteRes<[HIP12UnitALU14]> {
}

let Latency = 6, NumMicroOps = 1 in
def HIP12Write_6c_1LD : SchedWriteRes<[HIP12UnitLD]> {
}

let Latency = 6, NumMicroOps = 3 in
def HIP12Write_6c_1LD_2ALU : SchedWriteRes<[HIP12UnitLD, HIP12UnitALU]> {
  let ResourceCycles = [1, 2];
}

let Latency = 8, NumMicroOps = 1 in
def HIP12Write_8c_1LD : SchedWriteRes<[HIP12UnitLD]> {
}

let Latency = 8, NumMicroOps = 3 in
def HIP12Write_8c_2ALU_1LD : SchedWriteRes<[HIP12UnitALU, HIP12UnitLD]> {
  let ResourceCycles = [2, 1];
}

let Latency = 3, NumMicroOps = 2 in
def HIP12Write_3c_1ST_1STD : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD]> {
}

let Latency = 3, NumMicroOps = 5 in
def HIP12Write_3c_3ALU_1ST_1STD : SchedWriteRes<[HIP12UnitALU, HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [3, 1, 1];
}

let Latency = 5, NumMicroOps = 2 in
def HIP12Write_5c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 7, NumMicroOps = 3 in
def HIP12Write_7c_3V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [3];
}

let Latency = 9, NumMicroOps = 4 in
def HIP12Write_9c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 6, NumMicroOps = 2 in
def HIP12Write_6c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 9, NumMicroOps = 3 in
def HIP12Write_9c_3V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [3];
}

let Latency = 12, NumMicroOps = 4 in
def HIP12Write_12c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 4, NumMicroOps = 2 in
def HIP12Write_4c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 7, NumMicroOps = 2 in
def HIP12Write_7c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 9, NumMicroOps = 2 in
def HIP12Write_9c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 8, NumMicroOps = 4 in
def HIP12Write_8c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 6, NumMicroOps = 4 in
def HIP12Write_6c_2ALU0134_2V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [2, 2];
}

let Latency = 4, NumMicroOps = 3 in
def HIP12Write_4c_3V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [3];
}

let Latency = 6, NumMicroOps = 3 in
def HIP12Write_6c_3V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [3];
}

let Latency = 6, NumMicroOps = 4 in
def HIP12Write_6c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 6, NumMicroOps = 4 in
def HIP12Write_6c_2V_2ALU0134 : SchedWriteRes<[HIP12UnitV, HIP12UnitALU0134]> {
  let ResourceCycles = [2, 2];
}

let Latency = 7, NumMicroOps = 2 in
def HIP12Write_7c_2LD : SchedWriteRes<[HIP12UnitLD]> {
  let ResourceCycles = [2];
}

let Latency = 8, NumMicroOps = 3 in
def HIP12Write_8c_1LD_2V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [1, 2];
}

let Latency = 8, NumMicroOps = 4 in
def HIP12Write_8c_2LD_2V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [2, 2];
}

let Latency = 10, NumMicroOps = 11 in
def HIP12Write_10c_5LD_6V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [5, 6];
}

let Latency = 8, NumMicroOps = 6 in
def HIP12Write_8c_3LD_3V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [3, 3];
}

let Latency = 10, NumMicroOps = 14 in
def HIP12Write_10c_6LD_8V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [6, 8];
}

let Latency = 8, NumMicroOps = 7 in
def HIP12Write_8c_3LD_4V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [3, 4];
}

let Latency = 1, NumMicroOps = 4 in
def HIP12Write_1c_2ST_2STD : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [2, 2];
}

let Latency = 1, NumMicroOps = 4 in
def HIP12Write_1c_1ST_1STD_2V : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD, HIP12UnitV]> {
  let ResourceCycles = [1, 1, 2];
}

let Latency = 1, NumMicroOps = 8 in
def HIP12Write_1c_2ST_2STD_4V : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD, HIP12UnitV]> {
  let ResourceCycles = [2, 2, 4];
}

let Latency = 4, NumMicroOps = 4 in
def HIP12Write_4c_1ST_1STD_2V : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD, HIP12UnitV]> {
  let ResourceCycles = [1, 1, 2];
}

let Latency = 4, NumMicroOps = 8 in
def HIP12Write_4c_4V_2ST_2STD : SchedWriteRes<[HIP12UnitV, HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [4, 2, 2];
}

let Latency = 4, NumMicroOps = 4 in
def HIP12Write_4c_2V_1ST_1STD : SchedWriteRes<[HIP12UnitV, HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [2, 1, 1];
}

let Latency = 4, NumMicroOps = 1 in
def HIP12Write_4c_1V02 : SchedWriteRes<[HIP12UnitV02]> {
}

let Latency = 2, NumMicroOps = 1 in
def HIP12Write_2c_1V02 : SchedWriteRes<[HIP12UnitV02]> {
}

let Latency = 2, NumMicroOps = 2 in
def HIP12Write_2c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 6, NumMicroOps = 4 in
def HIP12Write_6c_1V02_3ALU : SchedWriteRes<[HIP12UnitV02, HIP12UnitALU]> {
  let ResourceCycles = [1, 3];
}

let Latency = 7, NumMicroOps = 2 in
def HIP12Write_7c_1V02_1ALU25 : SchedWriteRes<[HIP12UnitV02, HIP12UnitALU25]> {
}

let Latency = 3, NumMicroOps = 2 in
def HIP12Write_3c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 1, NumMicroOps = 2 in
def HIP12Write_1c_2V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [2];
}

let Latency = 4, NumMicroOps = 4 in
def HIP12Write_4c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 9, NumMicroOps = 4 in
def HIP12Write_9c_2ALU0134_2V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [2, 2];
}

let Latency = 7, NumMicroOps = 4 in
def HIP12Write_7c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 5, NumMicroOps = 4 in
def HIP12Write_5c_2ALU0134_2V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [2, 2];
}

let Latency = 10, NumMicroOps = 4 in
def HIP12Write_10c_2ALU0134_2V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [2, 2];
}

let Latency = 11, NumMicroOps = 4 in
def HIP12Write_11c_2ALU0134_2V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [2, 2];
}

let Latency = 11, NumMicroOps = 8 in
def HIP12Write_11c_4ALU0134_4V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [4, 4];
}

let Latency = 10, NumMicroOps = 8 in
def HIP12Write_10c_4ALU0134_4V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [4, 4];
}

let Latency = 10, NumMicroOps = 12 in
def HIP12Write_10c_6ALU0134_6V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [6, 6];
}

let Latency = 11, NumMicroOps = 16 in
def HIP12Write_11c_8ALU0134_8V : SchedWriteRes<[HIP12UnitALU0134, HIP12UnitV]> {
  let ResourceCycles = [8, 8];
}

let Latency = 5, NumMicroOps = 4 in
def HIP12Write_5c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 7, NumMicroOps = 6 in
def HIP12Write_7c_6V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [6];
}

let Latency = 8, NumMicroOps = 3 in
def HIP12Write_8c_3V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [3];
}

let Latency = 10, NumMicroOps = 8 in
def HIP12Write_10c_8V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [8];
}

let Latency = 11, NumMicroOps = 4 in
def HIP12Write_11c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 13, NumMicroOps = 10 in
def HIP12Write_13c_10V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [10];
}

let Latency = 8, NumMicroOps = 6 in
def HIP12Write_8c_6V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [6];
}

let Latency = 11, NumMicroOps = 8 in
def HIP12Write_11c_8V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [8];
}

let Latency = 14, NumMicroOps = 10 in
def HIP12Write_14c_10V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [10];
}

let Latency = 18, NumMicroOps = 8 in
def HIP12Write_18c_8V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [8];
}

let Latency = 42, NumMicroOps = 36 in
def HIP12Write_42c_36V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [36];
}

let Latency = 10, NumMicroOps = 4 in
def HIP12Write_10c_4V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [4];
}

let Latency = 26, NumMicroOps = 20 in
def HIP12Write_26c_20V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [20];
}

let Latency = 20, NumMicroOps = 24 in
def HIP12Write_20c_24V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [24];
}

let Latency = 15, NumMicroOps = 10 in
def HIP12Write_15c_10V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [10];
}

let Latency = 12, NumMicroOps = 8 in
def HIP12Write_12c_8V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [8];
}

let Latency = 9, NumMicroOps = 6 in
def HIP12Write_9c_6V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [6];
}

let Latency = 8, NumMicroOps = 8 in
def HIP12Write_8c_8V : SchedWriteRes<[HIP12UnitV]> {
  let ResourceCycles = [8];
}

let Latency = 10, NumMicroOps = 21 in
def HIP12Write_10c_9LD_12V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [9, 12];
}

let Latency = 11, NumMicroOps = 28 in
def HIP12Write_11c_12LD_16V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [12, 16];
}

let Latency = 11, NumMicroOps = 14 in
def HIP12Write_11c_6LD_8V : SchedWriteRes<[HIP12UnitLD, HIP12UnitV]> {
  let ResourceCycles = [6, 8];
}

let Latency = 9, NumMicroOps = 4 in
def HIP12Write_9c_4LD : SchedWriteRes<[HIP12UnitLD]> {
  let ResourceCycles = [4];
}

let Latency = 10, NumMicroOps = 8 in
def HIP12Write_10c_8LD : SchedWriteRes<[HIP12UnitLD]> {
  let ResourceCycles = [8];
}

let Latency = 8, NumMicroOps = 2 in
def HIP12Write_8c_2LD : SchedWriteRes<[HIP12UnitLD]> {
  let ResourceCycles = [2];
}

let Latency = 1, NumMicroOps = 4 in
def HIP12Write_1c_2V_1ST_1STD : SchedWriteRes<[HIP12UnitV, HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [2, 1, 1];
}

let Latency = 7, NumMicroOps = 24 in
def HIP12Write_7c_12V_6ST_6STD : SchedWriteRes<[HIP12UnitV, HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [12, 6, 6];
}

let Latency = 7, NumMicroOps = 32 in
def HIP12Write_7c_16V_8ST_8STD : SchedWriteRes<[HIP12UnitV, HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [16, 8, 8];
}

let Latency = 4, NumMicroOps = 8 in
def HIP12Write_4c_4ST_4STD : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [4, 4];
}

let Latency = 8, NumMicroOps = 16 in
def HIP12Write_8c_8ST_8STD : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [8, 8];
}

let Latency = 2, NumMicroOps = 4 in
def HIP12Write_2c_2ST_2STD : SchedWriteRes<[HIP12UnitST, HIP12UnitSTD]> {
  let ResourceCycles = [2, 2];
}

let Latency = 9, NumMicroOps = 54 in
def HIP12Write_9c_36V_18ALU25 : SchedWriteRes<[HIP12UnitV, HIP12UnitALU25]> {
  let ResourceCycles = [36, 18];
}