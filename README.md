# NAND Gate Simulator

## Overview

This program simulates a network of NAND gates, allowing users to create, connect, and evaluate logical circuits. It supports connecting NAND gates to other gates or constant boolean signals, managing their inputs and outputs, and evaluating the final signal states.

## Features

- **Create NAND gates** with a specified number of inputs.
- **Connect gates** to form complex logical circuits.
- **Attach constant boolean signals** as inputs.
- **Evaluate circuit behavior**, returning the output signal and depth of computation.
- **Memory management functions** to ensure proper cleanup of allocated structures.

## Algorithmic Solutions

- **Graph Representation**  
  The circuit is represented as a directed graph, where each NAND gate is a node, and connections are edges.

- **Graph Traversal (DFS)**  
  Evaluation of the circuit uses a **depth-first search (DFS)** approach to resolve dependencies and compute outputs.

- **Cycle Detection**  
  A visited flag prevents infinite loops caused by cyclic dependencies in the circuit.

- **Dynamic Memory Management**  
  The program uses dynamic memory allocation to efficiently handle gate creation and deletion.
