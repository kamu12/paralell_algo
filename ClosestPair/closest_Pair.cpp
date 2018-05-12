#include<iostream>
#include<future>
#include<thread>
#include<vector>
#include<algorithm>
#include<random>
#include<chrono>
#include<cmath>

using namespace std;
using namespace std::chrono;

const int PARALLEL_DEPTH = 2;//defines how many times we call the recursive func in paralell

struct Point
{
    double x, y;
};

struct Res{
    Point p1, p2;
    double dist;
};
 
int compX(const void* a, const void* b)
{
    Point *p1 = (Point *)a,  *p2 = (Point *)b;
    return (p1->x - p2->x);
}

int compY(const void* a, const void* b)
{
    Point *p1 = (Point *)a;
    Point *p2 = (Point *)b;
    return (p1->y - p2->y);
}
 
double dist(Point p1, Point p2)
{
    return sqrt( (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

Res seqRun(Point P[], int n)
{
    double min = INT32_MAX;
    Point p1,p2;
    for (int i = 0; i < n; ++i)
    {
        for (int j = i+1; j < n; ++j)
        {
            if (dist(P[i], P[j]) < min)
            {
                min = dist(P[i], P[j]);
                p1 = P[i];
                p2 = P[j];
            }
        }
    }

    return {p1, p2, min};
}
 
double min(double x, double y)
{
    return (x < y)? x : y;
}
 
Res stripClosest(Point strip[], int size, Res d)
{
    double min = d.dist;
    Point p1 = d.p1, p2 = d.p2;
    qsort(strip, size, sizeof(Point), compY); 
 
    for (int i = 0; i < size; ++i){
        for (int j = i+1; j < size && (strip[j].y - strip[i].y) < min; ++j)
        {
            if (dist(strip[i],strip[j]) < min)
            {
                min = dist(strip[i], strip[j]);
                p1 = strip[i];
                p2 = strip[j];
            }
        }
    }
    return {p1, p2, min};
}
 
Res closestIter(Point P[], int n)
{
    if (n <= 10)
        return seqRun(P, n);
 
    int mid = n/2;
    Point midPoint = P[mid];
 
    Res left = closestIter(P, mid);
    Res right = closestIter(P + mid, n-mid);
 
    double d = min(left.dist, right.dist);
    Point p1, p2;
    if(left.dist < right.dist)
    {
        p1 = left.p1;
        p2 = left.p2;
    }
    else
    {
        p1 = right.p1;
        p2 = right.p2;
    }

    Point strip[n];
    int j = 0;
    for (int i = 0; i < n; i++)
    {   
        if (abs(P[i].x - midPoint.x) < d)
        {
            strip[j] = P[i], j++;
        }
    }
    
    Res stripRes = stripClosest(strip, j, {p1, p2, d});
    if(d < stripRes.dist)
    {
        return {p1, p2, d};
    }
    else 
    {
        return stripRes;
    }
}

Res closestParIter(Point P[], int n, int depth)
{
    if (n <= 3)
        return seqRun(P, n);
 
    int mid = n/2;
    Point midPoint = P[mid];
 
    Res left, right;
    if(depth < PARALLEL_DEPTH)
    {
        future<Res> l = async(closestParIter, P, mid, depth+1);
        future<Res> r = async(closestParIter, P + mid, n-mid, depth+1);
        left = l.get();
        right = r.get();
    }
    else
    {
        left = closestIter(P, mid);
        right = closestIter(P + mid, n-mid);
    }
     
    double d = min(left.dist, right.dist);
    Point p1, p2;
    if(left.dist < right.dist)
    {
        p1 = left.p1;
        p2 = left.p2;
    }
    else
    {
        p1 = right.p1;
        p2 = right.p2;
    }
 
    Point strip[n];
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        if (abs(P[i].x - midPoint.x) < d)
        {
            strip[j] = P[i], j++;
        }
    }
 
    Res stripRes = stripClosest(strip, j, {p1, p2, d});
    if(d < stripRes.dist)
    {
        return {p1, p2, d};
    }
    else 
    {
        return stripRes;
    }
}
 
Res closest(Point P[], int n)
{
    qsort(P, n, sizeof(Point), compX);
    return closestIter(P, n);
}

Res closestPar(Point P[], int n, int depth)
{
    qsort(P, n, sizeof(Point), compX);
    return closestParIter(P, n, depth);
}
 
int main()
{
    random_device rd; 
    mt19937 gen(rd()); 
    uniform_real_distribution<> dis(1, 10000);
    int size = 10000;
    vector<Point> P(size);

    generate(P.begin(), P.end()-1, [&](){return Point{dis(gen), dis(gen)};});

    size_t n_threads = thread::hardware_concurrency();
    cout << "Number of threads: " << n_threads << endl;
    int n = P.size();
    high_resolution_clock::time_point startSeq = high_resolution_clock::now();
    Res res = closest(P.data(), n);
    high_resolution_clock::time_point endSeq = high_resolution_clock::now();

    high_resolution_clock::time_point startPar = high_resolution_clock::now();
    Res resPar = closestPar(P.data(), n, 0);
    high_resolution_clock::time_point endPar = high_resolution_clock::now();
    cout << "The smallest distance is " << res.dist << endl;
    cout << "The smallest distance in par is " << resPar.dist << endl;
    cout << "Points are (" << res.p1.x << ", " << res.p1.y <<") (" << res.p2.x << ", " << res.p2.y <<")" << endl;
    cout << "Points are (" << resPar.p1.x << ", " << resPar.p1.y <<") (" << resPar.p2.x << ", " << resPar.p2.y <<")" << endl;
    auto seqTime = duration_cast<microseconds>(endSeq - startSeq).count();
    auto parTime = duration_cast<microseconds>(endPar - startPar).count();

    cout << "Seq time " << seqTime << endl;
    cout << "Par time " << parTime << endl;
    cout << "Speed up " << (static_cast<double>(seqTime) / parTime) << endl;
    
    return 0;
}