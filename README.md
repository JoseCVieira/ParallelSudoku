# CPD

- sudoku-aux.h + sudoku-aux.c => funcoes de teste (a eliminar para versao final)
- input-file                  => ficheiro com o sudoku input

- sudoku_parser.py            => fetch sudoku from website (requires python3 and BeautifulSoup4)
  - python3 sudoku_parser.py http://www.menneske.no/sudoku/eng/random.html?diff=9

- sudoku-recursive.c          => comparacao de tempos
  - ./sudoku-recursive input_file

- sudoku-seria-loop.c         => loop de verificacao de erros
  - ./sudoku-serial-loop                         

- sudoku-serial.c             => versao iterativa
  - ./sudoku-serial input_file
