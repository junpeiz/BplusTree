/* 
 * created by Lu Jialin @ Zhejiang University
 *
 * inspired by the introdution in book <Database System Concepts> (sixth edition) by A.Silberschatz , H.F.Korth and S.Sudarshan
 *
 * intended for the final project of the course "database system" to create a miniSQL
 *
 * B+ tree is used for efficient manageing (indexing , ordering and updating data in storage component)  
 *
 */

#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#define Degree 4 
//every node can store Degree-1 values at most

#include <iostream>
#include <string>
#include <vector>

using namespace std;

template<typename T> class BplusTree;
template<typename T> class bpt_node;

template<typename T>
class bpt_node
{
public:
	bpt_node();
	bool is_leaf;
	bool is_root;
	int key_num;
	T key[Degree];
	void *pointer[Degree + 1];
	void *father;
	//use to save the bpt_tree
	int flag;
	int offset[Degree];
	int save_pointer[Degree + 1];
};

template<typename T>
class BplusTree
{
public:
	//consists of a B plus tree and index info
	bpt_node<T> * root;
	string table_name;
	string index_name;

	BplusTree();
	~BplusTree();
	
	//return a new b plus tree node with initial value
	bpt_node<T> * new_bpt_node();
	//find the leaf node of specified value 
	bpt_node<T> * Find(T value);
	void Insert(bpt_node<T> * node, T value, void* record);
	void Delete(bpt_node<T> * node, T value);
	bool Insert(T value, void* record);
	bool Delete(T value);
	void split(bpt_node<T> * node);
	void delete_tree(bpt_node<T> * start);
	void query_single(T value, vector<char*> &query_res);
	void query_range(T value1, T value2, vector<char*> &query_res);
	void print();
	void before_save();
};

template<typename T>
bpt_node<T>::bpt_node()
{
	is_leaf = false;
	is_root = false;
	key_num = 0;
	pointer[0] = NULL;
	father = NULL;
}

template<typename T>
bpt_node<T> * BplusTree<T>::new_bpt_node()
{
	bpt_node<T> *p = new bpt_node<T>();
	return p;
}

template<typename T>
BplusTree<T>::BplusTree()
{
	root = new_bpt_node();
	root->is_root = true;
	root->is_leaf = true;
}

template<typename T>
void BplusTree<T>::delete_tree(bpt_node<T> * start)
{
	if (start == NULL)
		return;
	if (start->is_leaf)
	{
		delete start;
		return;
	}
	for (int i = 0; i <= start->key_num; i++)
		delete_tree((bpt_node<T> *)start->pointer[i]);
}

template<typename T>
BplusTree<T>::~BplusTree()
{
	delete_tree(this->root);
}

template<typename T>
bpt_node<T> * BplusTree<T>::Find(T value)
{
	bpt_node<T> *now = root;
	while (!now->is_leaf)
	{
		for (int i = 0; i <= now->key_num; i++)
			if (i == now->key_num || value < now->key[i])
			{
 				now = (bpt_node<T> *)now->pointer[i];
				break;
			}
	}
	return now;
}

template<typename T>
void BplusTree<T>::split(bpt_node<T> * node)
{
	bpt_node<T> *new_node = new_bpt_node();
	int mid_key = node->key[Degree / 2];
	new_node->key_num = Degree - Degree / 2 ;
	for (int i = 0; i <= new_node->key_num; i++)
	{
		new_node->key[i] = node->key[i + (Degree / 2)];
		new_node->pointer[i] = node->pointer[i + (Degree / 2)];
	}
	new_node->pointer[new_node->key_num] = node->pointer[Degree];
	node->key_num = Degree / 2-1;

	if (node->is_leaf)
	{
		node->key_num++;
		new_node->pointer[0] = node->pointer[0];
		node->pointer[0] = new_node;
		new_node->is_leaf = true;
		mid_key = node->key[Degree / 2 ];
	}

	if(node->is_root)
	{
		node->is_root = false;
		root = new_bpt_node();
		root->is_root = true;
		root->key[0] = mid_key;
		root->pointer[0] = node;
		root->pointer[1] = new_node;
		root->key_num = 1;
		node->father = new_node->father = root;
		return;
	}
	else
	{
		if(!node->is_leaf)
		{

		for (int i = 1;i<new_node->key_num; i++)
		{
			new_node->key[i-1] = new_node->key[i];
			new_node->pointer[i-1] = new_node->pointer[i];
		}
		new_node->key_num--;
		}

		new_node->father = node->father;
		Insert((bpt_node<T> *)node->father, mid_key, (void *)new_node);
	}
}

template<typename T>
bool BplusTree<T>::Insert(T value, void* record)
{
	bpt_node<T> * node = Find(value);
	for (int i = 0; i < node->key_num; i++)
		if (node->key[i] == value)
			return false;
	Insert(node, value, record);
	return true;
}

template<typename T>
void BplusTree<T>::Insert(bpt_node<T> * node, T value, void* record)
{
	int x = 0;
	while (x < node->key_num && node->key[x] < value) 
		x++;
	for (int i = node->key_num; i > x; i--)
		node->key[i] = node->key[i - 1];
	for (int i = node->key_num + 1; i > x + 1; i--)
		node->pointer[i] = node->pointer[i - 1];
	node->key[x] = value;
	node->pointer[x + 1] = record;
	node->key_num++;
	if (node->key_num == Degree) 
		split(node);
}

template<typename T>
bool BplusTree<T>::Delete(T value)
{
	bpt_node<T> * node = Find(value);
	for (int i = 0; i < node->key_num; i++)
		if (node->key[i] == value){
			Delete(node, value);
			return true;
		}
	return false;
}

template<typename T>
void BplusTree<T>::Delete(bpt_node<T> * node, T value)
{
	int x = 0;
	while (value != node->key[x])
		x++;
	for (int i = x; i < node->key_num - 1; i++)
		node->key[i] = node->key[i + 1];
	for (int i = x + 1; i < node->key_num; i++)
		node->pointer[i] = node->pointer[i + 1];
	node->key_num--;
}

template<typename T>
void BplusTree<T>::query_single(T value, vector<char*> &query_res)
{
	query_res.clear();
	bpt_node<T> * temp = Find(value);
	for (int i = 0; i < temp->key_num; i++)
		if (value == temp->key[i]){
			query_res.push_back((char *)temp->pointer[i+1]);
			return;
		}
	if (temp->key_num >= 2){
		for (int i = 0; i < temp->key_num - 1; i++)
			if (value < temp->key[i] && value> temp->key[i + 1])
				query_res.push_back((char *)temp->pointer[i + 1]);
	}
	else
		query_res.push_back((char *)temp->pointer[1]);
}

template<typename T>
void BplusTree<T>::query_range(T value1, T value2, vector<char*> &query_res)
{
	query_res.clear();
	int flag1, flag2;
	bpt_node<T> * temp1 = Find(value1);
	bpt_node<T> * temp2 = Find(value2);
	for (int i = 0; i < temp1->key_num; i++)
		if (value1 >= temp1->key[i]){
			flag1 = i + 1;
			break;
		}
	bool flag = false;
	for (int i = 0; i < temp2->key_num - 1; i++)
		if (value2 <= temp2->key[i] && value2 > temp2->key[i+1]){
			flag2 = i + 1;
			flag = true;
			break;
		}
	if (!flag)
		flag2 = temp2->key_num;
	bpt_node<T> * temp = temp1;
	for (int i = flag1; i <= temp->key_num; i++)
		query_res.push_back((char *)temp->pointer[i]);
	temp = temp->pointer[0];
	while (temp != temp2)
	{
		for (int i = 1; i <= temp->key_num;i++)
			query_res.push_back((char *)temp->pointer[i]);
		temp = temp->pointer[0];
	}
	for (int i = 1; i <= flag2; i++)
		query_res.push_back((char *)temp->pointer[i]);
}

template<typename T>
void print_bpt(bpt_node<T> * node,int depth)
{
	if (node == NULL)
		return;
	if (node->is_leaf)
	{


			for(int j =0;j<=depth-1;j++)
				cout<<" ";
			cout<<" "<<" key number is "<<node->key_num<<endl;	
	for (int i = 0; i < node->key_num; i++)
		{
		for(int j =0;j<=depth;j++)
			cout<<" ";
		cout<<"-";
		cout << node->key[i] <<endl;
		
		}
	}
	else
	{

			for(int j =0;j<=depth-1;j++)
				cout<<" ";
			cout<<" "<<" key number is "<<node->key_num<<endl;	
	
		for (int i = 0; i <= node->key_num; i++)
		{
			for(int j =0;j<=depth;j++)
				cout<<" ";
			cout<<" "<<"key value is "<<node->key[i]<<endl;	
			print_bpt((bpt_node<T> *)node->pointer[i],depth+1);
			for(int j =0;j<=depth+1;j++)
				cout<<" ";
			cout<<"|"<<endl;
		}
	}
}

template<typename T>
void BplusTree<T>::print()
{
	cout<<"here starts the structure"<<endl;

	print_bpt<T>(root,0);
	cout<<endl;
}

int flag_para;

template<typename T>
void set_flag(bpt_node<T> * node)
{
	if (node == NULL)
		return;
	node->flag = flag_para;
	flag_para++;
	if (node->is_leaf)
		return;
	else
		for (int i = 0; i <= node->key_num; i++)
			set_flag((bpt_node<T> *)node->pointer[i]);
}

template<typename T>
void set_offset(bpt_node<T> * node)
{
	if (node == NULL)
		return;
	if (node->is_leaf){
		for (int i = 1; i <= node->key_num; i++){
			//TODO:Call buffer manager to calculate the offset of the record
		}
	}
	else
		for (int i = 0; i <= node->key_num; i++)
			set_offset((bpt_node<T> *)node->pointer[i]);
}

template<typename T>
void set_save_pointer(bpt_node<T> * node)
{
	if (node == NULL || node->is_leaf)
		return;
	for (int i = 0; i <= node->key_num; i++)
		node->save_pointer[i] = ((bpt_node<T> *)node->pointer[i])->flag;
	for (int i = 0; i <= node->key_num; i++)
		set_save_pointer((bpt_node<T> *)node->pointer[i]);
}

template<typename T>
void BplusTree<T>::before_save()
{
	flag_para = 0;
	set_flag<T>(root);
	set_offset<T>(root);
	set_save_pointer<T>(root);
}

#endif
