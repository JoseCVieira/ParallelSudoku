#include "pomp_lib.h"


extern struct ompregdescr omp_rd_5;

int POMP_MAX_ID = 6;

struct ompregdescr* pomp_rd_table[6] = {
  0,
  0,
  0,
  0,
  0,
  &omp_rd_5,
};
