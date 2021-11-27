# Task 5

This folder contains the code library we build for Thread1:Task5.

Notice that we have already implemented a non in-place updates and in-place recommendations in q4, here we construct a different version that has in-place updates and non in-place recommendations. We argue that this new version should be a little bit faster since recommendations are completed by coping embeddings from embeddingholders. And the experiments support our arguement, though the number of `slow_function` remains unchange, which dominates the total running time.

The main idea here to support non in-place recommendation is that we copy the two embeddingholders `users` and `items` immediately when the `iter_idx` increases. For more details, please refer to files `q5_optimizer.cc` and `q5_optimizer.h` in this folder and `Thread-1/q5.cc`.
