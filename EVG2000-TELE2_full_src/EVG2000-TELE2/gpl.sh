#!/bin/sh

find -type f|xargs grep -m 100000  '//.*[fF]oxconn.*' | sed 's/:.*//' >../fileout.txt
n=$(wc -l ../fileout.txt |awk '{print $1}')

for (( i = 1 ; i <=$n ; i++)) 
do
     filename=$(cat ../fileout.txt | sed -n "$i","$i"p);
     cp $filename "$filename".bak
     sed 's/[/][/].*[fF]oxconn.*//' < "$filename".bak > "$filename" ; 
     rm -rf "$filename".bak
     echo "$i";
     echo "$filename";
          
done
rm -rf ../fileout.txt


find -type f|xargs grep -m 100000 '/[/*].*[fF]oxconn.*/' | sed 's/:.*//' > fileout.txt
n=$(wc -l ../fileout.txt |awk '{print $1}')
for (( i = 1 ; i <=$n ; i++)) 
do
     filename=$(cat ../fileout.txt | sed -n "$i","$i"p);
     cp $filename "$filename".bak
     sed 's/[/][*].*[fF]oxconn.*[*][/]//' < "$filename".bak > "$filename" ;

     rm -rf "$filename".bak
     echo "$i";
     echo "$filename";
         
done
rm -rf ../fileout.txt
