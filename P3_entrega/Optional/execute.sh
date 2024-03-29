#! /bin/sh
PURPLE='\033[1;35m'
GREEN='\033[1;36m'
NC='\033[0m' # No Color

#This executable deletes the data files generated by the program because,
#as it always inserts the same information, if you run multiple times
#without deleting the files, there will be repetitions on the table
rm index_score.dat 
rm index_scorematch.dat
rm score.dat

make clean
make
clear

echo "\n${PURPLE}Part 21${NC}"
echo "\n${GREEN}In this part, we do not include the table test because it is the same that the one from the compulory part.${NC}"

echo "\n${PURPLE}Part 2${NC}"
echo "${GREEN}Again, this does not use the index, so it the same one from the compulsory part${NC}"

echo "\n${PURPLE}Part 3${NC}"
echo "${GREEN}Score a book given its title and set up an index for the table on the score field${NC}"
echo "./score_index "Tres Novelas Ejemplares" 100"
./score_index "Tres Novelas Ejemplares" 100
echo "\n./score_index "En Contacto: Gramatica en Accion" 100"
./score_index "En Contacto: Gramatica en Accion" 100
echo "\n./score_index "The Life of Lope de Vega 1562-1635" 31"
./score_index "The Life of Lope de Vega 1562-1635" 31

echo "\n${GREEN}Get all the books with an score using the previously created index${NC}"
echo "./suggest_index 100"
./suggest_index 100
echo "\n./suggest_index 31"
./suggest_index 31
#This should not return anything
echo "\n${GREEN}This should not have results${NC}"
echo "./suggest_index 70"
./suggest_index 70


echo "\n${GREEN}Given a book title, get all the books that have the same score.${NC}"
echo "./scorematch "Tres Novelas Ejemplares""
./scorematch "Tres Novelas Ejemplares"
