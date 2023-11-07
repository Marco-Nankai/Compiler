#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include <string>
#include <queue>
#include <iomanip>
#include <map>
using namespace std;
const int start = 1;
const int End = 0;
int newstart;
const int alpha_num = 26;
vector<vector<vector<int>>> NFA;
vector<vector<int>> DFA;
vector<int> is_end;
set<char> charset;
set<set<int>> Opt_DFA_sets;
vector<vector<int>> OptDFA;
vector<int> Opt_is_end;
class Node {
public:
    char obj;
    Node* Left;
    Node* Right;
    Node() {
        Left = Right = nullptr;
    }
    Node(char obj) {
        this->obj = obj;
        Left = Right = nullptr;
    }
    Node* father(Node* left, Node* right, char obj) {
        Node* fat = new Node(obj);
        fat->Left = left;
        fat->Right = right;
        return fat;
    };
    void print(Node* root, int width)
    {
        if (root->Right != nullptr)
            print(root->Right, width + 1);
        cout << setw(width * 3) << root->obj << endl;
        if (root->Left != nullptr)
            print(root->Left, width + 1);
        return;
    }
};
class NFANode {
public:
    int start;
    int end;
    NFANode* Left;
    NFANode* Right;
    NFANode() {
        Left = Right = nullptr;
        start = -1;
        end = -1;
    }
};
struct SetComparer {
    bool operator()(const set<int>& a, const set<int>& b) const {
        if (a.size() != b.size()) {
            return a.size() < b.size();
        }

        auto ita = a.begin();
        auto itb = b.begin();
        while (ita != a.end() && itb != b.end()) {
            if (*ita != *itb) {
                return *ita < *itb;
            }
            ++ita;
            ++itb;
        }
        return false;
    }
};
NFANode* VisitParseTree(vector<vector<vector<int>>>& NFA, Node* root);
set<int> Episonal_closure(int NfaNode);
set<int> Episonal_closure_set(set<int> NewNodeSet);
void GetDFA();
vector<vector<int>> Merge();
set<set<int>> MiniDFA();
set<int> CheckStateBelong(int state, set<int> oristate, set<set<int>> States);
bool is_SameStates(int state1, int state2, set<set<int>> States);
ostream& operator<<(ostream& out, set<int>& A) {
    cout << "{";
    for (auto it = A.begin(); it != A.end(); it++)
        cout << *it << ",";
    cout << "}" << endl;
    return out;
}
int cnt = 0;
int main() {
    string regular;
    cin >> regular;//只能输入|，*，()
    Node* root = new Node();
   
    //得到语法分析树遍历得到NFA
    stack<Node*> op;
    stack<Node*> letter;//闭包运算统一存在左结点
    for (int i = 0; i < regular.size(); i++)
    {
        if (regular[i] == ' ')
            continue;
        Node* objNode = new Node(regular[i]);
        if (regular[i] == '(')
        {
            op.push(objNode);
        }
        else if (regular[i] == ')')
        {
            while (op.top()->obj != '(')
            {
                //这个时候里面的运算符一定是连着的|或者是+
                char obj = op.top()->obj;
                op.pop();
                Node* right = letter.top();
                letter.pop();
                Node* left = letter.top();
                letter.pop();
                Node* newNode = right->father(left, right, obj);
                letter.push(newNode);
            }
            op.pop();//把(清空出去
        }
        else if (regular[i] == '|')
        {
            if (op.top()->obj == '|' || op.top()->obj == '(' || op.empty())
            {
                op.push(objNode);
                continue;
            }
            else
            {
                while (!op.empty() && op.top()->obj == '+')
                {
                    char obj = op.top()->obj;
                    op.pop();
                    Node* right = letter.top();
                    letter.pop();
                    Node* left = letter.top();
                    letter.pop();
                    Node* newNode = right->father(left, right, obj);
                    letter.push(newNode);
                }
                op.push(objNode);
            }
        }
        else if (regular[i] == '*')
        {
            Node* node = letter.top();
            Node* closureNode = node->father(node, nullptr, '*');
            letter.pop();
            letter.push(closureNode);
        }
        else
        {
            if (letter.empty())
            {
                letter.push(objNode);
            }
            else {
                Node* addNode = new Node('+');
                if (!op.size())
                {
                    op.push(addNode);
                    letter.push(objNode);
                    continue;
                }
                if (op.top()->obj == '+')
                {
                    op.push(addNode);
                    letter.push(objNode);
                }
                else if (op.top()->obj == '(')
                {
                    Node* leftNode = letter.top();
                    letter.pop();
                    addNode->Left = leftNode;
                    addNode->Right = objNode;
                    letter.push(addNode);
                }
                else if (op.top()->obj == '|')
                {
                    if (letter.size() == 1)
                    {
                        letter.push(objNode);
                    }
                    else
                    {
                        Node* leftNode = letter.top();
                        letter.pop();
                        addNode->Left = leftNode;
                        addNode->Right = objNode;
                        letter.push(addNode);
                    }
                }
            }
        }
    }
    while (op.size())
    {
        char obj = op.top()->obj;
        op.pop();
        Node* right = letter.top();
        letter.pop();
        Node* left = letter.top();
        letter.pop();
        Node* newNode = right->father(left, right, obj);
        letter.push(newNode);
    }
    root = letter.top();
    root->print(root, 0);

    NFANode* NFATree;
    NFATree = VisitParseTree(NFA, root);
    cout << endl << endl << endl;
    //现在已经成功构造NFA了，然后转换为DFA，然后再化简
    for (int i = 0; i < regular.size(); i++)
    {
        if (regular[i] >= 'a' && regular[i] <= 'z')
            charset.insert(regular[i]);
    }
    //下面开始根据输入可能的字符将NFA转为DFA，注意，我们的开始结点得根据语法分析树
    /*总结一下NFA转DFA的方法，首先是找没标记过的结点压到队列里
    * 然后找队列首结点的episinol闭包，完了就开始访问可能出现的字符
    * 如果访问出来是空集，说明没有这条边
    * 如果访问出来是一个没标记过的集合，那我们就得压到队列里
    * 直到我们的队列清空，那就完成了这个工作
    * 这个过程得记录DFA结点的去向
    */
    GetDFA();
    cout << endl << endl;
    for (int i = 0; i < DFA.size(); i++)
    {
        for (int j = 0; j < alpha_num; j++)
            cout << DFA[i][j] << " ";
        cout << endl;
    }
    for (int i = 0; i < is_end.size(); i++)
        cout << is_end[i] << " ";
    cout << endl;
    /*下面进行DFA的优化
    * 方法是首先分成两个状态组，一个初态组，一个末态组
    * 我们从初态组开始，我们首先有一个记录状态组路径的数据结构
    * 我们这样测试，取一个状态组，遍历里面所有状态
    * 如果里面所有状态对所有可能的字符的转移都在另一个状态组，那就是同一状态
    * 如果出现了状态不是，那就把它踢出去，然后去找它应该在的状态组。
    * 所以问题的关键之处在于，我们需要实现一个类或者函数，加上一个存储的数据结构
    * 它有这样的方法：f(set<int>,char) = set<int>
    */
    /*
    * 书上的测试代码
    set<int> init;
    set<int> end;
    init.insert(0);
    init.insert(1);
    init.insert(2);
    end.insert(3);
    end.insert(4);
    end.insert(5);
    cout << "开始初态集合：" << init << endl;
    cout << "开始终态集合：" << end << endl;
    DFA.clear();
    is_end.clear();
    vector<int> temp(26, -1);
    for (int i = 0; i < 6; i++) {
        DFA.push_back(temp);
        if (i == 3 || i == 4 || i == 5)
            is_end.push_back(1);
        else is_end.push_back(0);
    }
    DFA[0][0] = 1;
    DFA[0][1] = 0;
    DFA[1][0] = 1;
    DFA[1][1] = 2;
    DFA[2][0] = 1;
    DFA[2][1] = 3;
    DFA[3][0] = 4;
    DFA[3][1] = 3;
    DFA[4][0] = 4;
    DFA[4][1] = 5;
    DFA[5][0] = 4;
    DFA[5][1] = 3;
    Opt_DFA_sets = MiniDFA();
    for (auto it = Opt_DFA_sets.begin(); it != Opt_DFA_sets.end(); it++) {
        set<int> temp = *it;
        cout << temp << endl;
    }
    */
    return 0;
}
NFANode* VisitParseTree(vector<vector<vector<int>>>& NFA, Node* root) {
    NFANode* LeftNode = nullptr;
    NFANode* RightNode = nullptr;
    NFANode* retNode = new NFANode();
    retNode->end = cnt++;
    retNode->start = cnt++;
    int startindex = cnt - 1;
    int endindex = cnt - 2;
    vector<int> init(1,-1);
    NFA.push_back(vector<vector<int>>(27,init));//26个字母加空字符串
    NFA.push_back(vector<vector<int>>(27,init));
    if (root->Left != nullptr)
        LeftNode = VisitParseTree(NFA, root->Left);
    if (root->Right != nullptr)
        RightNode = VisitParseTree(NFA, root->Right);   
    retNode->Left = LeftNode;
    retNode->Right = RightNode;
    if (root->obj >= 'a' && root->obj <= 'z')
    {
        if (NFA[startindex][int(root->obj - 'a')][0] == -1)
            NFA[startindex][int(root->obj - 'a')][0] = endindex;
        else
            NFA[startindex][int(root->obj - 'a')].push_back(endindex);
    }
    else if (root->obj == '*')
    {
        int LeftNodeEnd = LeftNode->end;
        int LeftNodeStart = LeftNode->start;
        if (NFA[startindex][26][0] == -1)
            NFA[startindex][26][0] = LeftNodeStart;
        else
            NFA[startindex][26].push_back(LeftNodeStart);

        if (NFA[LeftNodeEnd][26][0] == -1)
            NFA[LeftNodeEnd][26][0] = LeftNodeStart;
        else
            NFA[LeftNodeEnd][26].push_back(LeftNodeStart);

        if (NFA[LeftNodeEnd][26][0] == -1)
            NFA[LeftNodeEnd][26][0] = endindex;
        else
            NFA[LeftNodeEnd][26].push_back(endindex);

        if (NFA[startindex][26][0] == -1)
            NFA[startindex][26][0] = endindex;
        else
            NFA[startindex][26].push_back(endindex);
    }
    else 
    {
        //现在cnt是下一行应该有的数字，也就是说cnt-1是最后一行
        //然后开始做连接
        int LeftNodeEnd = LeftNode->end;
        int LeftNodeStart = LeftNode->start;
        int RightNodeStart = RightNode->start;
        int RightNodeEnd = RightNode->end;
        if (root->obj == '+')
        {
            if (NFA[startindex][26][0] == -1)
                NFA[startindex][26][0] = LeftNodeStart;
            else
                NFA[startindex][26].push_back(LeftNodeStart);

            if (NFA[LeftNodeEnd][26][0] == -1)
                NFA[LeftNodeEnd][26][0] = RightNodeStart;
            else
                NFA[LeftNodeEnd][26].push_back(RightNodeStart);

            if (NFA[RightNodeEnd][26][0] == -1)
                NFA[RightNodeEnd][26][0] = endindex;
            else
                NFA[RightNodeEnd][26].push_back(endindex);
        }
        else if (root->obj == '|')
        {
            if (NFA[startindex][26][0] == -1)
                NFA[startindex][26][0] = LeftNodeStart;
            else
                NFA[startindex][26].push_back(LeftNodeStart);

            if (NFA[startindex][26][0] == -1)
                NFA[startindex][26][0] = RightNodeStart;
            else
                NFA[startindex][26].push_back(RightNodeStart);

            if (NFA[LeftNodeEnd][26][0] == -1)
                NFA[LeftNodeEnd][26][0] = endindex;
            else
                NFA[LeftNodeEnd][26].push_back(endindex);

            if (NFA[RightNodeEnd][26][0] == -1)
                NFA[RightNodeEnd][26][0] = endindex;
            else
                NFA[RightNodeEnd][26].push_back(endindex);
        }
    }
    for (int i = 0; i < NFA.size(); i++)
    {
        cout << i << ":   ";
        for (int j = 0; j < 27; j++)
            for (int k = 0; k < NFA[i][j].size(); k++)
            {
                if (k == NFA[i][j].size() - 1)
                    cout << NFA[i][j][k] << " ";
                else
                    cout << NFA[i][j][k] << ",";
            }
        cout << endl;
    }
    cout << endl << endl << endl;
    return retNode;
}
set<int> Episonal_closure(int NfaNode)
{
    set<int> Node_closure; 
    Node_closure.insert(NfaNode);
    queue<int> unsearched;
    unsearched.push(NfaNode);
    int searchNode;
    while(!unsearched.empty())
    {
        searchNode = unsearched.front();
        vector<int> episonal_arrival = NFA[searchNode][26];
        for (int j = 0; j < episonal_arrival.size(); j++) {
            if (episonal_arrival[j] == -1)
                break;
            else {
                if (Node_closure.find(episonal_arrival[j]) == Node_closure.end()) {
                    Node_closure.insert(episonal_arrival[j]);
                    unsearched.push(episonal_arrival[j]);
                }
            }
        }
        unsearched.pop();
    }
    return Node_closure;
}
set<int> Episonal_closure_set(set<int> NewNodeSet)
{
    set<int> ans;
    for (auto it = NewNodeSet.begin(); it != NewNodeSet.end(); it++)
    {
        set<int> temp = Episonal_closure(*it);
        for (auto tempit = temp.begin(); tempit != temp.end(); tempit++)
            ans.insert(*tempit);
    }
    return ans;
}
void GetDFA() {
    set<set<int>> Encoded_DFANode;//记录已经编过号的状态集合
    queue<set<int>> UnMatchedNode;//记录还没有用字符寻找过的状态集合
    map<set<int>, int, SetComparer> NodeMap;//记录状态集合的编号
    set<int> StartNodeSet = Episonal_closure(start);//开始状态集合
    int DFAcount = 0;//已经有的DFA结点个数
    Encoded_DFANode.insert(StartNodeSet);
    NodeMap[StartNodeSet] = DFAcount++;//给开始结点赋值为0
    UnMatchedNode.push(StartNodeSet);
    if (StartNodeSet.find(End) != StartNodeSet.end())
        is_end.push_back(1);
    else
        is_end.push_back(0);
    vector<int> start(alpha_num, -1);//这是记录新结点的边的数据结构
    DFA.push_back(start);
    while (!UnMatchedNode.empty()) {
        //遍历状态集里的每个元素，然后取episinal闭包
        set<int> CurNodeSet = UnMatchedNode.front();
        for (auto i = CurNodeSet.begin(); i != CurNodeSet.end(); i++)
            cout << *i << " ";
        cout << endl;
        UnMatchedNode.pop();
        set<int> NewNodeSet;
        for (auto i = charset.begin();i != charset.end();i++)
        {
            NewNodeSet.clear();
            int index = *i - 'a';
            for (auto it = CurNodeSet.begin(); it != CurNodeSet.end(); it++)
            {
                vector<int> Destination = NFA[*it][index];
                for (int j = 0; j < Destination.size(); j++)
                {
                    if (Destination[j] == -1)
                        break;
                    else
                        NewNodeSet.insert(Destination[j]);
                }
            }
            NewNodeSet = Episonal_closure_set(NewNodeSet);
            if (NewNodeSet.size() == 0)
                continue;
            if (Encoded_DFANode.find(NewNodeSet) == Encoded_DFANode.end())//没找到，可以插入
            {
                UnMatchedNode.push(NewNodeSet);
                Encoded_DFANode.insert(NewNodeSet);//表明已经找过
                NodeMap[NewNodeSet] = DFAcount++;
                DFA.push_back(start);
                if (NewNodeSet.find(End) != NewNodeSet.end())
                    is_end.push_back(1);
                else
                    is_end.push_back(0);
            }//边的初始化
            int startNodeNum = NodeMap[CurNodeSet];
            int endNodeNum = NodeMap[NewNodeSet];
            DFA[startNodeNum][index] = endNodeNum;//记录边
            cout << startNodeNum << "通过" << char(index + 'a') << "走" << endNodeNum << ":";
            for (auto i = NewNodeSet.begin(); i != NewNodeSet.end(); i++)
                cout << *i << " ";
            cout << endl;
        }
    }
}
set<set<int>> MiniDFA() {
    set<set<int>> States;
    set<int> init_start;
    set<int> init_end;
    for (int i = 0; i < DFA.size(); i++)
    {
        if (is_end[i])
            init_end.insert(i);
        else
            init_start.insert(i);
    }
    States.insert(init_start);
    States.insert(init_end);
    bool restart;
    while (true)
    {
        restart = false;
        bool statechange = false;
        for (auto Curset = States.begin(); Curset != States.end(); Curset++)//取状态集合里的集合
        {
            set<int> frontDes;
            if ((*Curset).size() == 1)//分裂出一个状态的时候已经检查了合并的问题了
                continue;
            for (auto alpha = charset.begin(); alpha != charset.end(); alpha++) 
            {
                for (auto state = (*Curset).begin(); state != (*Curset).end(); state++)
                {
                    //去找ele在哪个状态集合里
                    set<int> curDes;
                    int StateDesNum = DFA[*state][int(*alpha - 'a')];
                    for (auto Desset = States.begin(); Desset != States.end(); Desset++)
                    {
                        if ((*Desset).find(StateDesNum) != (*Desset).end()) {
                            curDes = *Desset;
                            break;
                        }
                    }
                    if (state == (*Curset).begin())
                    {
                        frontDes = curDes;
                        continue;
                    }//第一次
                    else
                    {
                        if (curDes == frontDes)
                            continue;//
                        else//和前面的不一样，应该分裂,注意我们应该让这个状态找到它的归宿，首先非终态和终态肯定不能合在一起了
                        {
                            //把Curset中的State裂开
                            set<int> newState = *Curset;
                            int statenum = *state;
                            newState.erase(statenum);//我们去找state的归宿,应该先分裂，再检查
                            States.erase(*Curset);
                            States.insert(newState);
                            set<int> temp;
                            temp.insert(statenum);//先插入分裂后的子集合和子元素
                            States.insert(temp);
                            set<int> belongstate = CheckStateBelong(statenum, newState, States);
                            if (belongstate.size() != 0)
                            {
                                States.erase(temp);
                                States.erase(belongstate);
                                belongstate.insert(statenum);
                                States.insert(belongstate);
                            }
                            statechange = true;
                            restart = true;
                        }
                    }
                    if (restart) break;
                }
                if (restart) break;
            }
            if (restart) break;
        }
        if (restart) continue;
        if (!statechange)
            break;
    }
    return States;
}
set<int> CheckStateBelong(int state,set<int> oristate, set<set<int>> States)
{
    set<int> ResultStates;
    for (auto curSet = States.begin(); curSet != States.end(); curSet++)
    {
        int state1 = *((*curSet).begin());
        if (state1 == state)
            continue;//查到才分裂的了
        if ((*curSet) == oristate)
            continue;//查到原来的集合了
        bool thesame = true;
        for (auto alpha = charset.begin(); alpha != charset.end(); alpha++)
        {
            if (!thesame) break;
            int des = DFA[state][int(*alpha - 'a')];
            int des1 = DFA[state1][int(*alpha - 'a')];//检查它两的目的边是不是在同一个状态组
            if ((des == -1 && des1 != -1) || (des1 == -1 && des != -1))
            {
                thesame = false;
                break;
            }
            if (des == -1 && des1 == -1)
                continue;
            thesame = is_SameStates(state, state1, States);
        }
        if (thesame)
        {
            ResultStates = (*curSet);
            break;
        }
    }
    return ResultStates;
}
bool is_SameStates(int state1, int state2,set<set<int>> States)
{
    bool found1 = false;
    bool found2 = false;
    for (auto state = States.begin(); state != States.end(); state++)
    {
        if ((*state).find(state1) != (*state).end())
            found1 = true;
        if ((*state).find(state2) != (*state).end())
            found2 = true;
        if (found1 == true && found2 == true)
            return true;
        if (found1 == true && found2 == false)
            return false;
        if (found2 == true && found1 == false)
            return false;
    }
}
vector<vector<int>> Merge() {
    if (OptDFA.size() == DFA.size()) {
        Opt_is_end = is_end;
        return DFA;
    }
    set<int> startset;
    map<set<int>, int> NewNode;
    vector<int> temp(26, -1);
    int DFAsize = DFA.size();
    int* OldMap = new int[DFAsize];
    int cnt = 0;
    for (auto newset = Opt_DFA_sets.begin(); newset != Opt_DFA_sets.end(); newset++)
    {
        if ((*newset).find(start) != (*newset).end())
        {
            NewNode[*newset] = cnt;
            startset = *newset;
            OptDFA.push_back(temp);
            for (auto oldstate = (*newset).begin(); oldstate != (*newset).end(); oldstate++)
                OldMap[*oldstate] = cnt;
            Opt_is_end.push_back(0);
            cnt++;
            break;
        }
    }
    for (auto newset = Opt_DFA_sets.begin(); newset != Opt_DFA_sets.end(); newset++)
    {
        if ((*newset) == startset)
            continue;
        NewNode[*newset] = cnt++;
        OptDFA.push_back(temp);
        Opt_is_end.push_back(0);
    }
    for (int i = 0; i < DFA.size(); i++)
    {
        for (int j = 0; j < 26; j++)
        {
            if (DFA[i][j] != -1)
            {
                int newstart = OldMap[i];
                int newdes = OldMap[DFA[i][j]];
                OptDFA[newstart][j] = newdes;
            }
        }
        if (is_end[i])
        {
            int newstart = OldMap[i];
            Opt_is_end[newstart] = 1;
        }
    }
    return OptDFA;
}