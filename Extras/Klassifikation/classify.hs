#! /opt/local/bin/runghc

import Data.List
import Data.Maybe
import System.IO.Unsafe
import System.Environment

remove :: (Eq a) => a -> [(a, b)] -> [(a, b)]
remove k = filter ((/= k) . fst)

apply :: (Eq a) => (b -> b) -> b -> a -> [(a, b)] -> [(a, b)]
apply f v0 k l = case lookup k l of
	Just v -> (k, f v) : remove k l
	Nothing -> (k, f v0) : l

readtable :: FilePath -> [(Double, Double, Double)]
readtable path = map ((\ [x0, x1, x2] -> (x0, x1, x2)) . map read . words) $ lines $ unsafePerformIO $ readFile path

table :: [(Double, Double, Double)]
table = readtable "normalized/color-green.txt"

mean l = sum l / fromIntegral (length l)
stddev l = sqrt $ (/ fromIntegral (length l)) $ sum $ map (\ x -> (x - mean_) ^ 2) l where
	mean_ = mean l
statist l = (minimum l, mean l, maximum l, stddev l)

toClass :: (Double, Double, Double) -> (Int, Int, Double)
toClass (x0, x1, x2) = if min_ == max_
	then (0, 0, x0)
	else (fromJust $ elemIndex min_ l, fromJust $ elemIndex max_ l, (mid_ - min_) / (max_ - min_))
	where
		l = [x0, x1, x2]
		min_ = minimum l
		max_ = maximum l
		mid_ = head $ l \\ [min_, max_]

classify :: [(Int, Int, Double)] -> [((Int, Int), [Double])]
classify = foldl' f [] where
	f l (i1, i2, x) = apply (x :) [] (i1, i2) l

main = do
	args <- getArgs
	let file = args !! 0
	putStr $ unlines $ map show $ map (\ (i, l) -> (i, statist l)) $ classify $ map toClass $ readtable file