#ifndef HEADER_PT
#define HEADER_PT

typedef unsigned short INT16U;

typedef struct
{
  INT16U lc;  
  INT16U dl;
}pt_dl;
typedef struct 
{
  INT16U lc;  
}pt;

#define PT_THREAD_WAITING   0
#define PT_THREAD_EXITED    1


//初始化任务变量，只在初始化函数中执行一次就行
#define PT_INIT(pt)     (pt)->lc = 0

//启动任务处理，放在函数开始处
#define PT_BEGIN(pt)    switch((pt)->lc) { case 0:

// 等待某个条件成立，若条件不成立则直接退出本函数，下一次进入本函数就直接跳到这个地方判断  
// __LINE__ 编译器内置宏，代表当前行号,比如:若当前行号为8,则 s = __LINE__; case __LINE__: 展开为 s = 8; case 8:
#define PT_WAIT_UNTIL(pt,condition)   (pt)->lc = __LINE__;   case __LINE__: \
                                      if(!(condition))  return             

// 结束任务,放在函数的最后
#define PT_END(pt)      } 

//
#define PT_LINE(pt) (pt)->lc = __LINE__;  case  __LINE__:
#define PT_NEXT(pt) (pt)->lc = __LINE__;    return; case  __LINE__:

#define PT_DELAY(pt,n)  (pt)->lc = __LINE__; (pt)->dl = 0; case  __LINE__: \
                                                if(((pt)->dl)++ < n) \
                                                  return
                                                      

// 等待某个条件不成立      
#define PT_WAIT_WHILE(pt,cond)    PT_WAIT_UNTIL((pt),!(cond)) 

// 等待某个子任务执行完成
#define PT_WAIT_THREAD(pt,child,thread)  (pt)->lc = __LINE__;   case __LINE__: \
                                          (*(thread))();                           \
                                          if(!((child)->lc == PT_THREAD_EXITED))  return
                                           
// 等待某个子任务执行完成
#define PT_WAIT_THREAD_S(pt,child,thread)  (pt)->lc = __LINE__;   case __LINE__: \
                                          (*(thread))(child);                           \
                                          if(!((child)->lc == PT_THREAD_EXITED))  return
                                          
// 新建一个子任务,并等待其执行完退出
#define PT_SPAWN(pt,child,thread) \
  PT_INIT((child));            \
  PT_WAIT_THREAD((pt),(child),(thread))

  // 新建一个子任务,并等待其执行完退出
#define PT_SPAWN_S(pt,child,thread) \
  PT_INIT((child));            \
  PT_WAIT_THREAD_S((pt),(child),(thread))
  
// 重新启动某任务执行
#define PT_RESTART(pt)  PT_INIT(pt); return 

// 任务后面的部分不执行,直接退出
#define PT_EXIT(pt)     (pt)->lc = PT_THREAD_EXITED;return 


#endif
