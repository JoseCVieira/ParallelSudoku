void check_row_box(int **grid, int row, int num, int *v_aux, int startRow) {
    int i;

    for(i = startRow; i < (startRow + r_size); i++)
        if(i != row){
            if(is_exist_row(grid, i, num))
                v_aux[i-startRow] = 1;
            else
                v_aux[i-startRow] = 0;
        }else
            v_aux[i-startRow] = 0;
}

void check_col_box(int **grid, int col, int num, int *v_aux, int startCol) {
    int i;
    
    for(i = startCol; i < (startCol + r_size); i++)
        if(i != col){
            if(is_exist_col(grid, i, num))
                v_aux[i-startCol] = 1;
            else
                v_aux[i-startCol] = 0;
        }else
            v_aux[i-startCol] = 0;
}

void certain_elements(int **grid) {
    int i, j, k, x, y, changed = 1, cont, start_x, start_y, row, col;
    int v_aux_c [r_size];
    int v_aux_r [r_size];
    
    while(changed){
        changed = 0;
        
        for(i = 0; i < m_size; i++)
            for(j = 0; j < m_size; j++)
                if(!grid[i][j])
                    for(k = 1; k <= m_size; k++)
                        if(is_safe_num(grid, i, j, k)){

                            start_x = i - (i % r_size);
                            start_y = j - (j % r_size);
                            
                            check_row_box(grid, i, k, v_aux_r, start_x);
                            check_col_box(grid, j, k, v_aux_c, start_y);
                            
                            cont = 0;
                            for(x = start_x; x < (start_x + r_size); x++){
                                if(!v_aux_r[x-start_x]){
                                    for(y = start_y; y < (start_y + r_size); y++){
                                        if(!v_aux_c[y-start_y]){
                                        
                                            if(!grid[x][y]){
                                                cont ++;
                                                row = i;
                                                col = j;
                                            }
                                            
                                            if(cont > 1)
                                                break;
                                        }
                                    }
                                if(cont > 1)
                                    break;
                                }
                            }
                            
                            if(cont == 1){
                                grid[row][col] = k;
                                changed = 1;
                                cont_test++;
                                printf("changed grid[%d][%d]=%d, cont=%d\n", row, col, k, cont_test);
                                break;
                            }
                        }
    }
}

int test_all_nums( char dir, int n, int grid[N][N] ){

    int num, cell, possible_pos, pos;
    int cell_not_filled = 0; //flag

    //test all numbers from 1 to N
    for( num = 1 ; num <= N ; num++ ){

        possible_pos = 0;
        switch(dir){
            
            case 'r':

                // if num is already present in the current row/col/box skip num
                if( is_exist_row(grid, n, num) ){ cell_not_filled++; continue; }

                //go through all cells of the row/col/box
                for( cell = 0 ; cell < N ; cell++ ){
                    
                    //if cell already has a number skip cell
                    if(grid[n][cell] != 0) continue; 

                    //if num is not in the column and not in the box of the cell: increment place_count
                    if( !is_exist_col( grid, cell, num) && !is_exist_box( grid, n - (n % 3), cell - (cell % 3), num) ){ 
                        
                        possible_pos++;
                        pos = cell;
                       
                        //if there is more than one place to put the current num, go to next num
                        if( possible_pos > 1 ) break;
                    }   
                }

                //if there is only one place to put the number add it to the matrix, go to next num
                if( possible_pos == 1 ) grid[n][pos] = num;
 
                else cell_not_filled ++;

                break;


            case 'c':

                // if num is already present in the current row/col/box skip num
                if( is_exist_col(grid, n, num) ){ cell_not_filled++; continue; }
                
                //go through all cells of the row/col/box
                for( cell = 0 ; cell < N ; cell++ ){
                    
                    //if cell already has a number skip cell
                    if( grid[cell][n] != 0 ) continue;

                    //if num is not in the column and not in the box of the cell: increment place_count
                    if( !is_exist_row( grid, cell, num ) && !is_exist_box( grid, cell - (cell % 3), n - (n % 3), num ) ){ 

                        possible_pos++;
                        pos = cell;
                       
                        //if there is more than one place to put the current num, go to next num
                        if( possible_pos > 1 )  break;
                    }   
                } 

                //if there is only one place to put the number add it to the matrix, go to next num
                if( possible_pos == 1 ) grid[pos][n] = num;
                else cell_not_filled ++;

                break;

          /*  case 'b':

                // if num is already present in the current row/col/box skip num
                if( is_exist_box(grid, n, (n%3)*3, num) ){ cell_not_filled++; continue; }
                
                //to convert from box numbering (index = 1...N) to x,y coordinates:
                // x = index % #cols   (#cols = sqrt(N))
                // y = index / #cols

                //go through all cells of the row/col/box
                for( cell = 0 ; cell < N ; cell++ ){
                    
                    //if cell already has a number skip cell
                    if( grid[cell/3][cell%3] != 0 ){ printf("cell has number\n"); continue;}

                    //if num is not in the column and not in the box of the cell: increment place_count
                    if( !is_exist_row( grid, cell, num ) && !is_exist_col( grid, cell, num ) ){ 

                        possible_pos++;
                        pos = cell;
                       
                        //if there is more than one place to put the current num, go to next num
                        if( possible_pos > 1 )  break;
                    }   
                } 

                //if there is only one place to put the number add it to the matrix, go to next num
                if( possible_pos == 1 ) grid[cell/3][cell%3] = num;
                else cell_not_filled ++;

                break; */
        }

        
    }

    if( cell_not_filled == N ) return 1; //no cell was filled
    return 0;
}

int find_by_( char dir, int grid[N][N] ){

    int num, n, finished;
    
    //test all (rows/columns/boxes)
    for( n = 0 ; n < N ; n++ ){
        
        //go through the current row/col/box for as long as at least a cell has been filled
        //try to find a cell for each number
        finished = 0;
        while( ! (finished = test_all_nums( dir, n, grid )) ); 
    }

    return 0;
}

int place_finding_method( int grid[N][N] ){

    int return_val = 0;

    return_val = find_by_( 'r', grid ); //find by row
    return_val = find_by_( 'c', grid ); //find by col
    //return_val = find_by_( 'b', grid ); //find by box

    return 0;
}