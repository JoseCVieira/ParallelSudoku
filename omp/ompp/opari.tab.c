#include "pomp_lib.h"


extern struct ompregdescr omp_rd_19;
extern struct ompregdescr omp_rd_20;
extern struct ompregdescr omp_rd_21;
extern struct ompregdescr omp_rd_22;
extern struct ompregdescr omp_rd_23;
extern struct ompregdescr omp_rd_24;
extern struct ompregdescr omp_rd_25;
extern struct ompregdescr omp_rd_26;
extern struct ompregdescr omp_rd_27;

int POMP_MAX_ID = 28;

struct ompregdescr* pomp_rd_table[28] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  &omp_rd_19,
  &omp_rd_20,
  &omp_rd_21,
  &omp_rd_22,
  &omp_rd_23,
  &omp_rd_24,
  &omp_rd_25,
  &omp_rd_26,
  &omp_rd_27,
};
