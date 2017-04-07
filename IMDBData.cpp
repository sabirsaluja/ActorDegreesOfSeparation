// ITP 365 Spring 2016
// HW7 - IMDB Search
// Name: Sabir Saluja
// Email: sabirsal@usc.edu
// Platform: Mac
//
// IDMBData.cpp - Implements IMDBData interface

#include "IMDBData.h"
#include <fstream>
#include <iostream>
#include <queue>

// Do for Part 1
// Function: Default Constructor
// Purpose: Loads in the actor/movie file specified,
// and populates the data into the appropriate hash maps.
// Also constructs the graph with actor relationships.
// Input: Name of file to load (by reference)
// Returns: Nothing
IMDBData::IMDBData(const std::string& fileName)
{
    //int actorCount = 0;
    std::ifstream fileStream(fileName);         // Input file stream
    
    if (fileStream.is_open()) {                 // Check if open
        while (!fileStream.eof()) {             // Go till end of file
            std::vector<std::string> movies;    // Vector for movies = value
            std::string actor;                  // Actor name = key
            std::getline(fileStream, actor);    // First line is always actor
            while (fileStream.peek() == '|') {  // Go till last movie for actor
                std::string movie;
                std::getline(fileStream, movie);
                movies.push_back(movie);
            }
            // std::cout << actor << " : " << movies.size() << std::endl;
            mActorsToMoviesMap[actor] = movies; // Add key value pair
        }
        
        for (auto i : mActorsToMoviesMap) {
            reverseMap(i.first, i.second);
        }
        
    }
    else {
        std::cout << "File isn't open." << std::endl;
    }
}

// Do for Part 1
// Function: reverseMap
// Purpose: This is a helper function that is used to generate
// the reverse map (movies to actors)
// Input: Name of actor (by reference), vector of their movies (by reference)
// Returns: Nothing
void IMDBData::reverseMap(const std::string& actorName, const std::vector<std::string>& movies)
{
    for (int i = 0; i < movies.size(); i++) {       // Iterate through vector
        std::string movieName = movies[i].substr(1);
        if (mMoviesToActorsMap.find(movieName) != mMoviesToActorsMap.end()) {   // If map contains movie
            mMoviesToActorsMap[movieName].push_back(actorName);                 // Add to vector
        }
        else {                                                                  // If no movie
            std::vector<std::string> actors;                                    // Create the vector
            actors.push_back(actorName);                                        // Add the actor
            mMoviesToActorsMap[movieName] = actors;                             // Add to map
        }
    }
}

// Do for Part 1
// Function: getMoviesFromActor
// Purpose: Given an actor's name, returns a vector containing their movies
// Input: Name of actor (by reference)
// Returns: Vector of movies (by reference)
std::vector<std::string>& IMDBData::getMoviesFromActor(const std::string& actorName)
{
    if (mActorsToMoviesMap.find(actorName) == mActorsToMoviesMap.end()) {
        return sEmptyVector;
    }
    return mActorsToMoviesMap[actorName];
}

// Do for Part 1
// Function: getActorsFromMovie
// Purpose: Given a movie's name, returns the name of its actors
// Input: Name of movie (by reference)
// Returns: Vector of actors (by reference)
std::vector<std::string>& IMDBData::getActorsFromMovie(const std::string& movieName)
{
    if (mMoviesToActorsMap.find(movieName) == mMoviesToActorsMap.end()) {
        return sEmptyVector;
    }
    return mMoviesToActorsMap[movieName];
}

// Do for Part 2
// Function: findRelationship
// Purpose: Tries to find a relationship between two actors, using a BFS
// and outputs (to cout) the result of the search.
// Input: Name of two actors to check for a relationship
// Returns: Nothing
void IMDBData::findRelationship(const std::string& fromActor, const std::string& toActor)
{
    // Make Node* for each actor
    ActorNode* toActorNode = mGraph.getActorNode(toActor);
    
    // Make boolean for found target
    bool pathFound = false;
    
    // If the graph doesn't have the actors
    if (!mGraph.containsActor(fromActor) || !mGraph.containsActor(toActor)) {
        if (!mGraph.containsActor(fromActor)) {
            std::cout << fromActor << " is not found." << std::endl;
        }
        if (!mGraph.containsActor(toActor)) {
            std::cout << toActor << " is not found." << std::endl;
        }
    }
    
    else {  // Graph has both actors
        
        std::queue<ActorNode*> bfsQueue;    // Create the BFS Queue
        bfsQueue.push(mGraph.getActorNode(fromActor));
        
        // While the Queue isn't empty
        while (!bfsQueue.empty()) {
            
            // Dequeue ITP node
            ActorNode* currentNode = bfsQueue.front();
            bfsQueue.pop();
            
            // If current node is the target node
            // the path is found
            if (currentNode == toActorNode) {
                pathFound = true;
                break;
            }
            else {
                // Set the current node to visited
                currentNode->mIsVisited = true;
                
                // Loop through the edges
                for (Edge* edge : currentNode->mEdges) {
                    
                    // If the other actor's visited is false
                    if ((edge->mOtherActor)->mIsVisited == false) {
                        
                        // Enqueue adjacent node
                        bfsQueue.push(edge->mOtherActor);
                        
                        // If the adjacent node path size is 0
                        if (edge->mOtherActor->mPath.size() == 0) {
                            
                            // Set the adjacent node's path equal to current path
                            edge->mOtherActor->mPath = currentNode->mPath;
                            
                            // Push back path info
                            PathPair newPath = PathPair(edge->mMovieName, edge->mOtherActor->mName);
                            edge->mOtherActor->mPath.push_back(newPath);
                        }
                    }
                }
            }
        }
        
        if (pathFound) {
            std::cout << "Found a relationship." << std::endl;
            std::cout << toActorNode->mPath.size() << " hops..." << std::endl;
            std::cout << fromActor << " was in..." << std::endl;
            for (const PathPair& path : toActorNode->mPath) {
                if (path.getActorName() == toActor) {
                    std::cout << path.getMovieName() << " with " << path.getActorName() << std::endl;
                }
                else {
                    std::cout << path.getMovieName() << " with " << path.getActorName() << " who was in..." << std::endl;
                }
            }

        }
        else {
            std::cout << "No movie relationship!" << std::endl;
        }
    }

	mGraph.clearVisited();
}

// For Part 2
// Function: createGraph
// Purpose: This helper function constructs the IMDBGraph from the movie to actors map
// Input: None
// Returns: Nothing
void IMDBData::createGraph()
{
	// DO NOT EDIT THIS FUNCTION
	// For every movie in the actors map...
	for (auto& p : mMoviesToActorsMap)
	{
		const std::string& movieName = p.first;
		// Get the actors for this movie
		const std::vector<std::string>& actors = mMoviesToActorsMap[movieName];

		// Make n^2 edges between these actors
		for (size_t i = 0; i < actors.size(); i++)
		{
			ActorNode* firstActor = mGraph.getActorNode(actors[i]);
			for (size_t j = i + 1; j < actors.size(); j++)
			{
				ActorNode* secondActor = mGraph.getActorNode(actors[j]);
				mGraph.createActorEdge(firstActor, secondActor, movieName);
			}
		}
	}
}

// Leave here! Do not edit!!! Use in part 1
// DO NOT REMOVE THIS LINE
std::vector<std::string> IMDBData::sEmptyVector;
