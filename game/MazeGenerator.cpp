#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include "MazeGenerator.h"
using namespace std;
#define MAX_DELAY 50

int getName(int room, int *parent);
void merge(int name1, int name2, int *parent);
void drawMaze( tWall *walls, int w, int size, ostream &out);

void generateMaze(int roomsWide, int roomsHigh, tWall *walls) {
	int size = roomsWide * roomsHigh;
	int parent[size];
	int room1 = 0, room2 = -1;
	int direction;
	int name1 = 0, name2 = 0;
	srand(time(NULL));

	for(int i = 0; i < size; ++i) {
		parent[i] = i;
	}
	for( int k = 0; k < size - 1; ++k ) {
		do {
			//Pick a wall
			room1 = rand() % size;
			do {
				direction = rand() % 4;
				if( direction == MAZE_NORTH ) {
					room2 = room1 + roomsWide;
				} else if( direction == MAZE_EAST ) {
					room2 = room1 + 1;
				} else if( direction == MAZE_SOUTH ) {
					room2 = room1 - roomsWide;
				} else {
					room2 = room1 - 1;
				}
			} while( room2 < 0 || room2 >= size ||
					 (!((room1 + 1) % roomsWide) && !(room2 % roomsWide)) ||
					 (!((room2 + 1) % roomsWide) && !(room1 % roomsWide)));
			name1 = getName(room1, parent);
			name2 = getName(room2, parent);
		} while( name1 == name2 );
		merge(name1, name2, parent);

		//Store wall
		if( direction == MAZE_NORTH ) {
			walls[room1].top = false;
            walls[room2].bottom = false;
		} else if( direction == MAZE_EAST ) {
			walls[room2].left = false;
            walls[room1].right = false;
		} else if( direction == MAZE_SOUTH ) {
			walls[room2].top = false;
            walls[room1].bottom = false;
		} else {
			walls[room1].left = false;
            walls[room2].right = false;
		}
	}
	cout << "\nMaze complete.\n";
	ofstream fout;
	fout.open("Maze.txt");
	if( fout.good() ) {
		drawMaze(walls, roomsWide, size, fout);
		cout << "Maze written to \"Maze.txt\".\n";
	} else {
		cout << "ERROR\n";
	}
    fout.close();

}

int getName(int room, int *parent) {
	while(parent[room] != room) {
		room = parent[room];
	}
	return room;
}

int mergeUp(int name1, int *parent) {
	//Simple solution for now
	if( parent[name1] == name1 ) {
		return name1;
	} else {
		parent[name1] = mergeUp(parent[name1], parent);
		return parent[name1];
	}
}

void merge(int name1, int name2, int *parent) {
	parent[name2] = mergeUp(name1, parent);
}

void drawMaze( tWall *walls, int w, int size, ostream &out) {
	//Draw walls
	int index = size - w;
	while(index >= 0 ) { //for( int index = 0; index < size; ++index ) {
		for( int x = 0; x < w; ++x ) {
			if( walls[index + x].top )
				out << "+--";
			else
				out << "+  ";
		}
		out << "+" << endl;
		for( int x = 0; x < w; ++x ) {
			if( walls[index + x].left )
				out << "|  ";
			else
				out << "   ";
		}
		out << "|" << endl;
		index -= w;
	}

	for( int i = 0; i < w; ++i ) {
		out << "+--";
	}
	out << "+\n";
}
