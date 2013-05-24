import Data.List (elemIndex, permutations)
 
{-players is a list of the players.
permutations is a list of triplets, one for each player p: 
(p, p's wonderwall, p's ninja assassin)-}
validOrderings::(Show a, Ord a) => [a] -> [(a,a,a)] -> [[a]]
validOrderings players preferences = filter validOrder (permutations players)
    where
        validOrder possible = not (elem False 
            (map (\ (x, y, z) -> ((compare (elemIndex x possible) (elemIndex y possible)) == 
                (compare (elemIndex y possible) (elemIndex z possible)))) preferences))
 
main = do
         putStr $ (show results) ++ "\n"
         putStr $ (show results2) ++ "\n"
       where
          results = validOrderings ['a','b','c','d','e','f'] [('a','b','c'),('b','c','d'),('c','b','a'),('d','b','a'),('e','a','d'),('f','c','e')]
          results2 = validOrderings [1,2,3,4,5,6,7] [(1,2,3),(2,4,6),(3,4,7),(4,1,5),(5,6,3),(6,1,7),(7,2,5)]


{- Example usage:
validOrderings ['a','b','c','d','e','f'] [('a','b','c'),('b','c','d'),('c','b','a'),('d','b','a'),('e','a','d'),('f','c','e')]
outputs ["eabcdf","fdcbae","dfcbae","eabcfd"]

validOrderings [1,2,3,4,5,6,7] [(1,2,3),(2,4,6),(3,4,7),(4,1,5),(5,6,3),(6,1,7),(7,2,5)]  -}
