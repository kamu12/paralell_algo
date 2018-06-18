
#include <stdio.h>
#include <vector>
#include <future>
#include <chrono>
#include <random>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace std::chrono;

struct Edge
{
    int src;
    int dest;
    int weight;
};

struct Graph
{
    int V; //number of vertices
    int E; //number of edges

    Edge *edge;
};

struct subset
{
    int parent;
    int rank;
};

int find(struct subset subsets[], int i);
void Union(struct subset subsets[], int x, int y);
void find_smallest(subset *subsets, Edge *edge, int i, int *cheapest);

void parallelBoruvkaMST(struct Graph *graph)
{
    int V = graph->V;
    int E = graph->E;
    Edge *edge = graph->edge;

    // creating V subsets.
    struct subset *subsets = new subset[V];

    // the cheapest edge of each subset
    int *cheapest = new int[V];

    // init
    for (int v = 0; v < V; ++v)
    {
        subsets[v].parent = v;
        subsets[v].rank = 0;
        cheapest[v] = -1;
    }

    int numTrees = V;
    int MSTweight = 0;
    std::vector<std::future<void>> results;
    while (numTrees > 1)
    {
        for (int v = 0; v < V; ++v)
        {
            cheapest[v] = -1;
        }
        
        for (int i = 0; i < E; i++)
        {
            results.emplace_back(async(find_smallest, subsets, edge, i, cheapest));
        }

        for (auto &&result : results)
        {
            result.get();
        }
        results.clear();

        for (int i = 0; i < V; i++)
        {
            if (cheapest[i] != -1)
            {
                int set1 = find(subsets, edge[cheapest[i]].src);
                int set2 = find(subsets, edge[cheapest[i]].dest);

                if (set1 == set2)
                {
                    continue;
                }
                MSTweight += edge[cheapest[i]].weight;
                printf("Edge %d-%d included in MST\n",
                       edge[cheapest[i]].src, edge[cheapest[i]].dest);

                Union(subsets, set1, set2);
                numTrees--;
            }
        }
    }

    printf("Weight of MST is %d\n", MSTweight);
    return;
}

void find_smallest(subset *subsets, Edge *edge, int i, int *cheapest)
{
    int set1 = find(subsets, edge[i].src);
    int set2 = find(subsets, edge[i].dest);

    if (set1 == set2)
    {
        return;
    }
    else
    {
        if (cheapest[set1] == -1 || edge[cheapest[set1]].weight > edge[i].weight)
        {
            cheapest[set1] = i;
        }

        if (cheapest[set2] == -1 || edge[cheapest[set2]].weight > edge[i].weight)
        {
            cheapest[set2] = i;
        }
    }
}

struct Graph *createGraph(int V, int E)
{
    Graph *graph = new Graph;
    graph->V = V;
    graph->E = E;
    graph->edge = new Edge[E];
    return graph;
}

//path compression
int find(struct subset subsets[], int i)
{
    if (subsets[i].parent != i)
    {
        subsets[i].parent = find(subsets, subsets[i].parent);
    }

    return subsets[i].parent;
}

// union by rank
void Union(struct subset subsets[], int x, int y)
{
    int xroot = find(subsets, x);
    int yroot = find(subsets, y);

    if (subsets[xroot].rank < subsets[yroot].rank)
    {
        subsets[xroot].parent = yroot;
    }
    else if (subsets[xroot].rank > subsets[yroot].rank)
    {
        subsets[yroot].parent = xroot;
    }
    else
    {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}

int main()
{
    int V = 9; // Number of vertices in graph
    int E = 17; // Number of edges in graph
    struct Graph *graph = createGraph(V, E);

    graph->edge[0].src = 0;
    graph->edge[0].dest = 5;
    graph->edge[0].weight = 1;

    graph->edge[1].src = 0;
    graph->edge[1].dest = 3;
    graph->edge[1].weight = 6;

    graph->edge[2].src = 1;
    graph->edge[2].dest = 3;
    graph->edge[2].weight = 7;

    graph->edge[3].src = 1;
    graph->edge[3].dest = 2;
    graph->edge[3].weight = 5;

    graph->edge[4].src = 2;
    graph->edge[4].dest = 5;
    graph->edge[4].weight = 10;

    graph->edge[5].src = 3;
    graph->edge[5].dest = 4;
    graph->edge[5].weight = 8;

    graph->edge[6].src = 4;
    graph->edge[6].dest = 5;
    graph->edge[6].weight = 5;

    graph->edge[7].src = 1;
    graph->edge[7].dest = 6;
    graph->edge[7].weight = 4;

    graph->edge[8].src = 2;
    graph->edge[8].dest = 4;
    graph->edge[8].weight = 11;

    graph->edge[9].src = 2;
    graph->edge[9].dest = 7;
    graph->edge[9].weight = 17;

    graph->edge[10].src = 5;
    graph->edge[10].dest = 8;
    graph->edge[10].weight = 3;

    graph->edge[11].src = 4;
    graph->edge[11].dest = 8;
    graph->edge[11].weight = 13;

    graph->edge[12].src = 7;
    graph->edge[12].dest = 8;
    graph->edge[12].weight = 14;

    graph->edge[13].src = 4;
    graph->edge[13].dest = 7;
    graph->edge[13].weight = 16;

    graph->edge[14].src = 3;
    graph->edge[14].dest = 6;
    graph->edge[14].weight = 15;

    graph->edge[15].src = 4;
    graph->edge[15].dest = 6;
    graph->edge[15].weight = 18;

    graph->edge[16].src = 6;
    graph->edge[16].dest = 7;
    graph->edge[16].weight = 9;

    parallelBoruvkaMST(graph);

    return 0;
}
