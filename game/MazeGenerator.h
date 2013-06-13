/*
 * MazeGenerator.h
 * Defines the functions and structures necessary for the maze generator
 */

#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

typedef enum {
	MAZE_NORTH,
	MAZE_EAST,
	MAZE_SOUTH,
	MAZE_WEST
} Dirs;

typedef struct sWall{
	bool top, bottom,
         left, right;
	sWall() {top = bottom = left = right = true;}
} tWall;


void generateMaze(int roomsWide, int roomsHigh, tWall *walls);

 #endif
