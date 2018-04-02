#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define FALSE 0
#define TRUE 1
#define ROW(i) i/m_size
#define COL(i) i%m_size

int r_size, m_size, v_size;

typedef struct Vertex {
    int num;         
    int cell; 
    int visited;  
} Vertex;

Vertex* new_vertex(int num, int cell);
int vertex_visited(Vertex **vertex);
int get_next_v(int curr_cell, int *vect);
int get_prev_v(int curr_cell, int prev_cell, int *vect );

