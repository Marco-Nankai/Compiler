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
    cin >> regular;//ֻ������|��*��()
    Node* root = new Node();
   
    //�õ��﷨�����������õ�NFA
    stack<Node*> op;
    stack<Node*> letter;//�հ�����ͳһ��������
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
                //���ʱ������������һ�������ŵ�|������+
                char obj = op.top()->obj;
                op.pop();
                Node* right = letter.top();
                letter.pop();
                Node* left = letter.top();
                letter.pop();
                Node* newNode = right->father(left, right, obj);
                letter.push(newNode);
            }
            op.pop();//��(��ճ�ȥ
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
    //�����Ѿ��ɹ�����NFA�ˣ�Ȼ��ת��ΪDFA��Ȼ���ٻ���
    for (int i = 0; i < regular.size(); i++)
    {
        if (regular[i] >= 'a' && regular[i] <= 'z')
            charset.insert(regular[i]);
    }
    //���濪ʼ����������ܵ��ַ���NFAתΪDFA��ע�⣬���ǵĿ�ʼ���ø����﷨������
    /*�ܽ�һ��NFAתDFA�ķ�������������û��ǹ��Ľ��ѹ��������
    * Ȼ���Ҷ����׽���episinol�հ������˾Ϳ�ʼ���ʿ��ܳ��ֵ��ַ�
    * ������ʳ����ǿռ���˵��û��������
    * ������ʳ�����һ��û��ǹ��ļ��ϣ������Ǿ͵�ѹ��������
    * ֱ�����ǵĶ�����գ��Ǿ�������������
    * ������̵ü�¼DFA����ȥ��
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
    /*�������DFA���Ż�
    * ���������ȷֳ�����״̬�飬һ����̬�飬һ��ĩ̬��
    * ���Ǵӳ�̬�鿪ʼ������������һ����¼״̬��·�������ݽṹ
    * �����������ԣ�ȡһ��״̬�飬������������״̬
    * �����������״̬�����п��ܵ��ַ���ת�ƶ�����һ��״̬�飬�Ǿ���ͬһ״̬
    * ���������״̬���ǣ��ǾͰ����߳�ȥ��Ȼ��ȥ����Ӧ���ڵ�״̬�顣
    * ��������Ĺؼ�֮�����ڣ�������Ҫʵ��һ������ߺ���������һ���洢�����ݽṹ
    * ���������ķ�����f(set<int>,char) = set<int>
    */
    /*
    * ���ϵĲ��Դ���
    set<int> init;
    set<int> end;
    init.insert(0);
    init.insert(1);
    init.insert(2);
    end.insert(3);
    end.insert(4);
    end.insert(5);
    cout << "��ʼ��̬���ϣ�" << init << endl;
    cout << "��ʼ��̬���ϣ�" << end << endl;
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
    NFA.push_back(vector<vector<int>>(27,init));//26����ĸ�ӿ��ַ���
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
        //����cnt����һ��Ӧ���е����֣�Ҳ����˵cnt-1�����һ��
        //Ȼ��ʼ������
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
    set<set<int>> Encoded_DFANode;//��¼�Ѿ�����ŵ�״̬����
    queue<set<int>> UnMatchedNode;//��¼��û�����ַ�Ѱ�ҹ���״̬����
    map<set<int>, int, SetComparer> NodeMap;//��¼״̬���ϵı��
    set<int> StartNodeSet = Episonal_closure(start);//��ʼ״̬����
    int DFAcount = 0;//�Ѿ��е�DFA������
    Encoded_DFANode.insert(StartNodeSet);
    NodeMap[StartNodeSet] = DFAcount++;//����ʼ��㸳ֵΪ0
    UnMatchedNode.push(StartNodeSet);
    if (StartNodeSet.find(End) != StartNodeSet.end())
        is_end.push_back(1);
    else
        is_end.push_back(0);
    vector<int> start(alpha_num, -1);//���Ǽ�¼�½��ıߵ����ݽṹ
    DFA.push_back(start);
    while (!UnMatchedNode.empty()) {
        //����״̬�����ÿ��Ԫ�أ�Ȼ��ȡepisinal�հ�
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
            if (Encoded_DFANode.find(NewNodeSet) == Encoded_DFANode.end())//û�ҵ������Բ���
            {
                UnMatchedNode.push(NewNodeSet);
                Encoded_DFANode.insert(NewNodeSet);//�����Ѿ��ҹ�
                NodeMap[NewNodeSet] = DFAcount++;
                DFA.push_back(start);
                if (NewNodeSet.find(End) != NewNodeSet.end())
                    is_end.push_back(1);
                else
                    is_end.push_back(0);
            }//�ߵĳ�ʼ��
            int startNodeNum = NodeMap[CurNodeSet];
            int endNodeNum = NodeMap[NewNodeSet];
            DFA[startNodeNum][index] = endNodeNum;//��¼��
            cout << startNodeNum << "ͨ��" << char(index + 'a') << "��" << endNodeNum << ":";
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
        for (auto Curset = States.begin(); Curset != States.end(); Curset++)//ȡ״̬������ļ���
        {
            set<int> frontDes;
            if ((*Curset).size() == 1)//���ѳ�һ��״̬��ʱ���Ѿ�����˺ϲ���������
                continue;
            for (auto alpha = charset.begin(); alpha != charset.end(); alpha++) 
            {
                for (auto state = (*Curset).begin(); state != (*Curset).end(); state++)
                {
                    //ȥ��ele���ĸ�״̬������
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
                    }//��һ��
                    else
                    {
                        if (curDes == frontDes)
                            continue;//
                        else//��ǰ��Ĳ�һ����Ӧ�÷���,ע������Ӧ�������״̬�ҵ����Ĺ��ޣ����ȷ���̬����̬�϶����ܺ���һ����
                        {
                            //��Curset�е�State�ѿ�
                            set<int> newState = *Curset;
                            int statenum = *state;
                            newState.erase(statenum);//����ȥ��state�Ĺ���,Ӧ���ȷ��ѣ��ټ��
                            States.erase(*Curset);
                            States.insert(newState);
                            set<int> temp;
                            temp.insert(statenum);//�Ȳ�����Ѻ���Ӽ��Ϻ���Ԫ��
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
            continue;//�鵽�ŷ��ѵ���
        if ((*curSet) == oristate)
            continue;//�鵽ԭ���ļ�����
        bool thesame = true;
        for (auto alpha = charset.begin(); alpha != charset.end(); alpha++)
        {
            if (!thesame) break;
            int des = DFA[state][int(*alpha - 'a')];
            int des1 = DFA[state1][int(*alpha - 'a')];//���������Ŀ�ı��ǲ�����ͬһ��״̬��
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