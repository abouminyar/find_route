#include <iostream>
#include <fstream>
#include <string>
#include <ctime>// ضفتها حتى نقدر نقارن الوقت

using namespace std;

const int MAX_CITIES = 50;// أقصى عدد مدن
const int MAX_ROADS = 15;// أقصى عدد طرق لكل مدينة
const int MAX_NODES = 10000;// حجم مخزن العقد
const int INF = 1000000000;// قيمة تمثل infinity

// يمثل طريق واحد
struct Road
{
    int toCity;// المدينة الوجهة
    int length;// طول الطريق
};

// يمثل عقدة في شجرة البحث
struct Node
{
    int city;// المدينة
    int parent;// رقم العقدة الأب (-1 للجذر)
    int cost;// المسافة من البداية
};

// نتيجة أي خوارزمية
struct Result
{
    bool found;// هل وجدنا طريق؟
    int distance;// المسافة الكلية
    int pathLength;// عدد المدن في المسار
    int path[MAX_CITIES];// المدن بالترتيب
    int expanded;// عدد العقد الموسعة
    int generated;// عدد العقد المولدة
    int maxFrontier;// أكبر حجم للـ Frontier
};

// ---------------------------------------------------
// Class 1: خريطة الطرق (Encapsulation)
// ---------------------------------------------------
class Graph
{
private:
    string names[MAX_CITIES];
    Road roads[MAX_CITIES][MAX_ROADS];
    int roadCount[MAX_CITIES];
    int cityCount;

    // إضافة طريق في اتجاه واحد
    void addRoad(int from, int to, int length)
    {
        roads[from][roadCount[from]].toCity = to;
        roads[from][roadCount[from]].length = length;
        roadCount[from]++;
    }

    // يرجع رقم المدينة، وإذا مش موجودة يضيفها
    int getOrAddCity(string name)
    {
        int index = findCity(name);
        if(index != -1){
            return index;
        }
        names[cityCount] = name;
        roadCount[cityCount] = 0;
        cityCount++;
        return cityCount - 1;
    }

public:
    // Constructor
    Graph()
    {
        cityCount = 0;
    }

    // قراءة الملف إلى أن نوصل END OF INPUT
    bool loadFile(string fileName)
    {
        ifstream file(fileName.c_str());
        if(!file.is_open()){
            return false;
        }
        string from, to;
        int length;
        while(file >> from){
            if(from == "END"){
                break;
            }
            file >> to >> length;
            int a = getOrAddCity(from);
            int b = getOrAddCity(to);
            // الطرق ثنائية الاتجاه
            addRoad(a, b, length);
            addRoad(b, a, length);
        }
        return true;
    }

    // رقم المدينة أو -1
    int findCity(string name)
    {
        for(int i=0;i<cityCount;i++){
            if(names[i] == name){
                return i;
            }
        }
        return -1;
    }

    string getName(int city){ return names[city]; }
    int getCityCount(){ return cityCount; }
    int getRoadCount(int city){ return roadCount[city]; }
    Road getRoad(int city, int i){ return roads[city][i]; }

    // طول الطريق المباشر بين مدينتين
    int getLength(int a, int b)
    {
        for(int i=0;i<roadCount[a];i++){
            if(roads[a][i].toCity == b){
                return roads[a][i].length;
            }
        }
        return INF;
    }
};

// ---------------------------------------------------
// Class 2: الأب لكل الخوارزميات (Abstract Class)
// ---------------------------------------------------
class Search
{
protected:
    Graph *graph;
    Node pool[MAX_NODES];// مخزن العقد
    int poolSize;

    // نتيجة فارغة
    Result emptyResult()
    {
        Result r;
        r.found = false;
        r.distance = INF;
        r.pathLength = 0;
        r.expanded = 0;
        r.generated = 0;
        r.maxFrontier = 0;
        return r;
    }

    // إضافة عقدة جديدة وترجيع رقمها
    int newNode(int city, int parent, int cost)
    {
        pool[poolSize].city = city;
        pool[poolSize].parent = parent;
        pool[poolSize].cost = cost;
        poolSize++;
        return poolSize - 1;
    }

    // بناء المسار من الهدف رجوعاً للبداية
    void buildPath(int goalNode, Result &r)
    {
        int temp[MAX_CITIES];
        int count = 0;
        int current = goalNode;
        while(current != -1){
            temp[count++] = pool[current].city;
            current = pool[current].parent;
        }
        r.pathLength = count;
        for(int i=0;i<count;i++){
            r.path[i] = temp[count - 1 - i];
        }
        r.found = true;
        r.distance = pool[goalNode].cost;
    }

public:
    // Constructor
    Search(Graph *g)
    {
        graph = g;
        poolSize = 0;
    }

    // Virtual Destructor
    virtual ~Search(){}

    // دالة البحث، كل خوارزمية تكتبها بطريقتها (Polymorphism)
    virtual Result search(int start, int goal) = 0;
};

// ---------------------------------------------------
// Class 3: BFS (Queue)
// ---------------------------------------------------
class BFS : public Search
{
public:
    BFS(Graph *g) : Search(g){}

    Result search(int start, int goal)
    {
        Result r = emptyResult();
        poolSize = 0;
        bool visited[MAX_CITIES] = {false};
        int queue[MAX_NODES];
        int head = 0, tail = 0;

        queue[tail++] = newNode(start, -1, 0);
        r.generated = 1;
        visited[start] = true;

        while(head < tail){
            int index = queue[head++];
            Node node = pool[index];
            r.expanded++;

            if(node.city == goal){
                buildPath(index, r);
                return r;
            }
            for(int i=0;i<graph->getRoadCount(node.city);i++){
                Road road = graph->getRoad(node.city, i);
                if(!visited[road.toCity]){
                    visited[road.toCity] = true;
                    queue[tail++] = newNode(road.toCity, index, node.cost + road.length);
                    r.generated++;
                }
            }
            if(tail - head > r.maxFrontier){
                r.maxFrontier = tail - head;
            }
        }
        return r;
    }
};

// ---------------------------------------------------
// Class 4: DFS (Stack)
// ---------------------------------------------------
class DFS : public Search
{
public:
    DFS(Graph *g) : Search(g){}

    Result search(int start, int goal)
    {
        Result r = emptyResult();
        poolSize = 0;
        bool visited[MAX_CITIES] = {false};
        int stack[MAX_NODES];
        int top = 0;

        stack[top++] = newNode(start, -1, 0);
        r.generated = 1;

        while(top > 0){
            int index = stack[--top];
            Node node = pool[index];

            // ممكن نفس المدينة تنحط مرتين، نتجاهل المكرر
            if(visited[node.city]){
                continue;
            }
            visited[node.city] = true;
            r.expanded++;

            if(node.city == goal){
                buildPath(index, r);
                return r;
            }
            // بالعكس حتى أول جار يتم استكشافه أولاً
            for(int i=graph->getRoadCount(node.city)-1;i>=0;i--){
                Road road = graph->getRoad(node.city, i);
                if(!visited[road.toCity]){
                    stack[top++] = newNode(road.toCity, index, node.cost + road.length);
                    r.generated++;
                }
            }
            if(top > r.maxFrontier){
                r.maxFrontier = top;
            }
        }
        return r;
    }
};

// ---------------------------------------------------
// Class 5: IDT (Iterative Deepening Tree Search)
// ---------------------------------------------------
class IDT : public Search
{
private:
    int path[MAX_CITIES];// المسار الحالي
    int pathLength;

    // هل المدينة موجودة على المسار الحالي؟
    bool onPath(int city)
    {
        for(int i=0;i<pathLength;i++){
            if(path[i] == city){
                return true;
            }
        }
        return false;
    }

    // بحث بعمق محدود
    bool depthLimited(int city, int goal, int limit, int cost, Result &r)
    {
        path[pathLength++] = city;
        r.expanded++;
        if(city == goal){
            r.found = true;
            r.distance = cost;
            r.pathLength = pathLength;
            for(int i=0;i<pathLength;i++){
                r.path[i] = path[i];
            }
            return true;
        }
        if(limit > 0){
            for(int i=0;i<graph->getRoadCount(city);i++){
                Road road = graph->getRoad(city, i);
                if(onPath(road.toCity)){
                    continue;
                }
                r.generated++;
                if(depthLimited(road.toCity, goal, limit - 1, cost + road.length, r)){
                    return true;
                }
            }
        }
        pathLength--;// رجوع للخلف
        return false;
    }

public:
    IDT(Graph *g) : Search(g)
    {
        pathLength = 0;
    }

    Result search(int start, int goal)
    {
        Result r = emptyResult();
        // نكرر البحث بعمق 0,1,2,...
        for(int limit=0;limit<graph->getCityCount();limit++){
            pathLength = 0;
            if(depthLimited(start, goal, limit, 0, r)){
                r.maxFrontier = limit + 1;
                return r;
            }
        }
        return r;
    }
};

// ---------------------------------------------------
// Class 6: UCS (أقل تكلفة أولاً)
// ---------------------------------------------------
class UCS : public Search
{
public:
    UCS(Graph *g) : Search(g){}

    Result search(int start, int goal)
    {
        Result r = emptyResult();
        poolSize = 0;
        bool closed[MAX_CITIES] = {false};
        int frontier[MAX_NODES];
        int size = 0;

        frontier[size++] = newNode(start, -1, 0);
        r.generated = 1;

        while(size > 0){
            // نبحث عن أقل تكلفة في الـ Frontier
            int best = 0;
            for(int i=1;i<size;i++){
                if(pool[frontier[i]].cost < pool[frontier[best]].cost){
                    best = i;
                }
            }
            int index = frontier[best];
            frontier[best] = frontier[--size];
            Node node = pool[index];

            if(closed[node.city]){
                continue;
            }
            closed[node.city] = true;
            r.expanded++;

            // الـ Goal Test عند التوسيع لضمان الحل الأمثل
            if(node.city == goal){
                buildPath(index, r);
                return r;
            }
            for(int i=0;i<graph->getRoadCount(node.city);i++){
                Road road = graph->getRoad(node.city, i);
                if(!closed[road.toCity]){
                    frontier[size++] = newNode(road.toCity, index, node.cost + road.length);
                    r.generated++;
                }
            }
            if(size > r.maxFrontier){
                r.maxFrontier = size;
            }
        }
        return r;
    }
};

// ---------------------------------------------------
// Class 7: الطباعة
// ---------------------------------------------------
class Printer
{
public:
    void printResult(Graph &g, Result &r)
    {
        if(!r.found){
            cout << "distance: infinity" << endl;
            cout << "Route: none" << endl;
            return;
        }
        cout << "distance: " << r.distance << " km" << endl;
        cout << "route:" << endl;
        for(int i=0;i+1<r.pathLength;i++){
            int a = r.path[i];
            int b = r.path[i+1];
            cout << g.getName(a) << " to " << g.getName(b)
                 << ", " << g.getLength(a, b) << " km" << endl;
        }
    }

    // الإحصائيات على cerr حتى ما تخرب الـ Output المطلوب
    void printStats(Result &r, double time)
    {
        cerr << "expanded: " << r.expanded << endl;
        cerr << "generated: " << r.generated << endl;
        cerr << "max_frontier: " << r.maxFrontier << endl;
        cerr << "time: " << time << " sec" << endl;
    }
};

int main(int argc, char *argv[])
{
    // لازم 4 Arguments
    if(argc != 5){
        cerr << "Usage: find_route algorithm input_filename origin_city destination_city\n";
        cerr << "algorithm: BFS | DFS | IDT | UCS\n";
        return 1;
    }

    string algorithm = argv[1];
    string fileName = argv[2];
    string origin = argv[3];
    string destination = argv[4];

    Graph graph;
    if(!graph.loadFile(fileName)){
        cerr << "Error: cannot open file " << fileName << endl;
        return 1;
    }

    int start = graph.findCity(origin);
    int goal = graph.findCity(destination);
    if(start == -1 || goal == -1){
        cerr << "Error: unknown city name\n";
        return 1;
    }

    // Pointer من نوع الأب يشير لأي خوارزمية (Polymorphism)
    Search *s = NULL;
    if(algorithm == "BFS"){
        s = new BFS(&graph);
    }
    else if(algorithm == "DFS"){
        s = new DFS(&graph);
    }
    else if(algorithm == "IDT"){
        s = new IDT(&graph);
    }
    else if(algorithm == "UCS"){
        s = new UCS(&graph);
    }
    else{
        cerr << "Error: unknown algorithm " << algorithm << endl;
        return 1;
    }

    Printer printer;

    // البحث سريع جداً والـ clock() ما يقدر يقيسه مرة واحدة،
    // فنكرر البحث RUNS مرة ونقسم الوقت على العدد (متوسط الوقت)
    const int RUNS = 10000;
    Result result;
    clock_t startTime = clock();
    for(int i=0;i<RUNS;i++){
        result = s->search(start, goal);
    }
    double elapsed = double(clock() - startTime) / CLOCKS_PER_SEC / RUNS;

    printer.printResult(graph, result);
    printer.printStats(result, elapsed);

    delete s;
    return 0;
}
