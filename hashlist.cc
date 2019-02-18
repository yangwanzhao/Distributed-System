#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <functional>
#include <boost/filesystem.hpp>
#include <libgen.h>
#include <unistd.h>
#include <boost/asio.hpp>
#include <mutex>

#include "hashlist.h"

using namespace std;
vector<mutex> mtx(HASH_NUM_MAX);

/*initial a hashlist*/
pHash_List init_hash_list(void)
{
  u32 i;
  pHash_List plist;
    //pHash_Header phead;

  plist = (Hash_List *)malloc(sizeof(Hash_List)); 

  for( i = 0;i < HASH_NUM_MAX;i++ )
  {
    plist->list[i] = (Hash_Header *)malloc(sizeof(Hash_Header)); 
    plist->list[i]->next = NULL;
  }     

  return plist;
}

/*insert a node by id*/
string insert_node_to_hash(pHash_List plist, string data)
{
  Node *ptail,*pre,*p;
  u32 id, position, len_key, len_value;
  string key, value, response;
  // (u32) len_key
  position = data.find("\n");
  len_key = stoi(data.substr(0, position));
  data = data.substr(position+1);
  // (string) key
  position = data.find("\n");
  key = data.substr(0, position);
  data = data.substr(position+1);
  // (u32) len_value

  position = data.find("\n");
  len_value = stoi(data.substr(0, position));
  // (string) value 
  value = data.substr(position+1);

  std::hash<std::string> h;
  size_t n = h(key);
  id = n % HASH_NUM_MAX;

  // cout << "#####test2: " << len_key << " " << key << " " << len_value << " " << value << " " << id << endl;

  // ptail = (Node *)malloc(sizeof(Node));
  ptail = new Node;
  ptail->next = NULL;
  ptail->id   = id;
  ptail->len_key = len_key;
  ptail->len_value = len_value;
  ptail->key   = key;
  ptail->value = value;
  


  while(true)
  {
    if (mtx[id].try_lock())
    {
      if( NULL == plist->list[id]->next )
      {
        plist->list[id]->next = ptail;
        response = "OK";
        mtx[id].unlock();
        return response;
      }

      pre = plist->list[id]->next;
      while( pre )
      {
        if (pre->key == ptail->key)
        {
          response = "ERROR";
          mtx[id].unlock();
          return response;
        }
        p = pre;
        pre = pre->next;   
      }
      p->next = ptail;
      response = "OK";

      mtx[id].unlock();
      break;
    }
    
  }
  return response;
}

/*delete a node by id*/
string delete_node_to_hash(pHash_List plist,string data)
{
  Node *psea;
  u32 id, position, len_key;
  string key, response;

  // (u32) len_key
  position = data.find("\n");
  len_key = stoi(data.substr(0, position));
  // (string) key
  key = data.substr(position+1);
  
  std::hash<std::string> h;
  size_t n = h(key);
  id = n % HASH_NUM_MAX;


  while(true)
  {
    if (mtx[id].try_lock())
    {
      psea = plist->list[id]->next; 
      if( NULL == psea )
      {
       response = "ERROR";
       mtx[id].unlock();
       return response;
     } 
     if( key == psea->key )
     {
       plist->list[id]->next = psea->next; 
       free(psea);
       response = "OK";
       mtx[id].unlock();
       return response;
     }
     if( NULL == psea->next )
     {
       response = "ERROR";
       mtx[id].unlock();
       return response; 
     } 

     while( key != psea->next->key )
     {
       psea = psea->next;
       if( NULL == psea->next )
       {
         response = "ERROR";
         mtx[id].unlock();
         return response;
       }       
     } 
     psea->next = psea->next->next;
     free(psea->next);

     response = "OK";

     mtx[id].unlock();
     break;
   }

 }
 return response;
}


string get_node_to_hash(pHash_List plist, string data)
{
  Node *psea;
  u32 id, position, len_key;
  string key, response;

  // (u32) len_key
  position = data.find("\n");
  len_key = stoi(data.substr(0, position));
  // (string) key
  key = data.substr(position+1);

  std::hash<std::string> h;
  size_t n = h(key);
  id = n % HASH_NUM_MAX;


  while(true)
  {
    if (mtx[id].try_lock())
    {
      psea = plist->list[id]->next; 
      if( NULL == psea )
      {
        response = "ERROR";
        mtx[id].unlock();
        return response;
      }

      if(key == psea->key )
      {
        response = "OK\nRESULT-LEN:" + to_string(psea->len_value) + "\nRESULT:" + psea->value;
        mtx[id].unlock();
        return response; 
      } 
      if( NULL == psea->next )
      {
        response = "ERROR";
        mtx[id].unlock();
        return response;
      }

      while( key != psea->next->key )
      {
        psea = psea->next;
        if( NULL == psea->next )
        {
         response = "ERROR";
         mtx[id].unlock();
         return response;
       }   
     }

     response = "OK\nRESULT-LEN:" + to_string(psea->len_value) + "\nRESULT:" + psea->value;
     mtx[id].unlock();
     return response;  
   }
 }

}

// ----------------------- JUST FOR TEST -------------
void init_hash(pHash_List plist)
{
  string data;
  data = "3\nwww\n2\nqq";
  insert_node_to_hash(plist, data);
  data = "4\nwaaa\n5\nqaaaq";
  insert_node_to_hash(plist, data);
  data = "5\nqavqw\n7\nqasdfsq";
  insert_node_to_hash(plist, data);
  data = "4\nqaqw\n2\nqa";
  insert_node_to_hash(plist, data);
  data = "6\nqaa2qw\n4\nq22a";
  insert_node_to_hash(plist, data);
}
// ----------------------- JUST FOR TEST -------------


/*print the whole hash table*/
void print_hash(pHash_List plist)
{
  u32 i;
  pNode p; 

  cout << "Print the hash table:" << endl; 

  for( i = 0;i < HASH_NUM_MAX;i++)
  {
    p = plist->list[i]->next;

    while( NULL != p )
    {
      cout << "id=" << p->id << " len_key=" << p->len_key << " key=" << p->key << " len_value=" << p->len_value << " value=" << p->value << endl;
      p = p->next;
    }        
  }
  cout << endl;
} 

/*free the whole hash table*/
// never used 
void free_all_hash(pHash_List plist)
{
  u32 i;
  pNode p,pn; 

  cout << "Free the whole hashtable" << endl; 

  for( i = 0;i < HASH_NUM_MAX;i++)
  {
    p = plist->list[i]->next;;
    pn = p;
    if(NULL == p)
    {
      continue;
    }   
    while( NULL != pn )
    {
      p = pn;
      pn = p->next;

      cout << "id=" << p->id << " key=" << p->key << " value=" << p->value << endl;  
      free(p);              
            //p = p->next;
    }      
    free(plist->list[i]);  
  }

} 

