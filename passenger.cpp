#include<bits/stdc++.h>
using namespace std;

class Passenger
{
    string name;
    int id;
    public:
    Passenger(string n,int i)
    {
        name=n;
        id=i;
    }
    string getname()
    {
        return name;
    }
   
    void displaypassenger()
    {
        cout<<endl<<"Passenger Name : "<<name<<endl;
        cout<<"ID : "<<id<<endl;
    }
    double calculatefare(double fare)
    {
        return fare;
    }
};

    class RegularPassenger : public Passenger
    {
        public:
        RegularPassenger(string n,int i):Passenger(n,i)
        {
        }
        double calculatefare(double fare)
        {
            return fare;
        }
    };

    class StudentPassenger : public Passenger
    {
        public:
        StudentPassenger(string n,int i):Passenger(n,i)
        {
        }
        double calculatefare(double fare)
        {
            return fare*0.5;
        }
    };


class Ticket
{
    int ticket_id;
    string source;
    string destination;
    int fare;
    public:
    Ticket(int id,string s,string d,int f)
    {
        ticket_id=id;
        source=s;
        destination=d;
        fare=f;
    }
  
    void displayticket()
    {
        cout<<endl<<"-----METRO TICKET-----"<<endl;
        cout<<"Ticket ID : "<<ticket_id<<endl;
        cout<<"Source : "<<source<<endl;
        cout<<"Destination : "<<destination<<endl;
        cout<<"Fare : "<<fare<<endl;
        cout<<"----------------------"<<endl;
    }

};


class Station
{
    string stn_name;
    int stn_id;
    public:
    Station()
    {
        stn_id=0;
        stn_name="";
    }
    Station(int id,string n)
    {
        stn_id=id;
        stn_name=n;
    }
    int getid()
    {
        return stn_id;
    }
    string getname()
    {
        return stn_name;
    }
    void displaystation()
    {
        cout<<stn_id<<" : "<<stn_name<<endl;
    }
};


class Passengerqueue
{
    Passenger* queue[20];
    int front, rear;
    public:
    Passengerqueue()
    {
        front=-1;
        rear=-1;
    }
    void enqueue(Passenger* p)
    {
        if (rear==19)
        {
            cout<<"Queue is full !";
            return;
        }
        if (front==-1)
            front=0;
        rear++;
        queue[rear]=p;
        cout<<"Passenger in queue "<<endl;
    }
    void dequeue()
    {
        if (front==-1 || front>rear)
        {
            cout<<"Queue is empty "<<endl;
            return;
        }
        cout<<"Passenger entered metro "<<endl;
        queue[front]->displaypassenger();
        delete queue[front];
        front++;
    }
};


class action_stk
{
    string stk[20];
    int top;
    public:
    action_stk()
    {
        top=-1;
    }
    void push(string action)
    {
        if (top==19)
        {
            cout<<"Stack is full !";
            return;
        }
        top++;
        stk[top]=action;
    }
    void pop()
    {
        if (top==-1)
        {
            cout<<"Stack is empty !";
            return;
        }
        cout<<"Last action : "<<stk[top]<<endl;
        top--;
    }
    void display()
    {
        if (top==-1)
        {
            cout<<"No actions available !"<<endl;
            return;
        }
        cout<<"Actions performed : "<<endl;
        for (int i=top;i>=0;i--)
        {
            cout<<stk[i]<<endl;
        }
    }
};


class metro
{
    Station s[10];
    int stn_count;
    Passengerqueue pq;
    action_stk as;
    int tkt_count;
    Passenger* tktpassenger[20]; 
    int tp_count;
    public:
    metro()
    {
        stn_count=0;
        tkt_count=100;
        tp_count=0;
    }
    void addstation(int id,string name )
    {
        if (stn_count==10)
        {
            cout<<"Max no. of stations reached !"<<endl;
            return;
        }
        s[stn_count]=Station(id,name);
        stn_count++;
    }
    void displaystations()
    {
        cout<<"------  METRO STATIONS  ------ "<<endl;
        for (int i=0;i<stn_count;i++)
        {
            s[i].displaystation();
            cout<<endl;
        }
        cout<<"-------------------------------"<<endl;
    }
    int findstation(string name)
    {
        for (int i=0;i<stn_count;i++)
        {
            if (s[i].getname()==name)
                return s[i].getid();
        }
        return -1;
    }

    void regular_tkt(RegularPassenger* p,string source,string destination)
    {
        int src_id=findstation(source);
        int dest_id=findstation(destination);
        if (src_id==-1 || dest_id==-1)
        {
            cout<<"Invalid source or destination !"<<endl;
            return;
        }
        double fare=abs(src_id-dest_id)*10;
        if(fare==0)
        {
            cout<<"Source and destination cannot be same !"<<endl;
            return;
        }
        double final_fare=p->calculatefare(fare);
        Ticket t(tkt_count,source,destination,final_fare);
        tkt_count++;
        t.displayticket();
        tktpassenger[tp_count]=p;
        tp_count++;
        as.push("Regular passenger "+p->getname()+" bought ticket from "+source+" to "+destination+" with fare Rs. "+to_string(final_fare));
    }

    void student_tkt(StudentPassenger* p,string source,string destination)
    {
        int src_id=findstation(source);
        int dest_id=findstation(destination);
        if (src_id==-1 || dest_id==-1)
        {
            cout<<"Invalid source or destination !"<<endl;
            return;
        }
        double fare=abs(src_id-dest_id)*10;
        if(fare==0)
        {
            cout<<"Source and destination cannot be same !"<<endl;
            return;
        }
        double final_fare=p->calculatefare(fare);
        Ticket t(tkt_count,source,destination,final_fare);
        tkt_count++;
        t.displayticket();
        tktpassenger[tp_count]=p;
        tp_count++;
        as.push("Student passenger "+p->getname()+" bought ticket from "+source+" to "+destination+" with fare Rs. "+to_string(final_fare));
    }
    void addpassenger()
    {
        if(tp_count==0) 
        {
             cout<<"No passenger has bought a ticket yet !"<<endl; 
             return; 
        }
        pq.enqueue(tktpassenger[0]);
        as.push("Passenger "+tktpassenger[0]->getname()+" added to queue");
        for(int i=0;i<tp_count-1;i++) 
        {
             tktpassenger[i]=tktpassenger[i+1]; 
        } 
        tp_count--;
    }
    void enter_metro()
    {
        pq.dequeue();
        as.push("Passenger entered metro");
    }
    void displayactions()
    {
        as.display();
    }
};


int main()
{
    metro m;
    m.addstation(1,"kashmiri gate");
    m.addstation(2,"rajiv chowk");
    m.addstation(3,"dwarka");
    m.addstation(4,"hauz khas");
    m.addstation(5,"noida city center");
    m.addstation(6,"nazibabad");
    m.addstation(7,"lajpat nagar");
    int choice;
    do
    {
        cout<<endl<<"--------- METRO TRANSIT MANAGER ---------"<<endl;
        cout<<"1. Display stations"<<endl;
        cout<<"2. Search a station"<<endl;
        cout<<"3. Generate ticket"<<endl;
        cout<<"4. Add passenger"<<endl;
        cout<<"5. Enter metro"<<endl;
        cout<<"6. Display recent actions"<<endl;
        cout<<"7. Exit"<<endl;
        cout<<"Enter your choice : ";
        cin>>choice;
        cout<<endl;
        switch(choice)
        {
            case 1:
            {
                m.displaystations();
            }
            break;
            case 2:
            {
                string name;
                cout<<"Enter station name  : ";
                cin.ignore();
                getline(cin,name);
                int id=m.findstation(name);
                if (id==-1)
                    cout<<"Station not found !"<<endl;
                else
                {
                    cout<<"Station ID : "<<id<<endl;
                    cout<<"Station Name : "<<name<<endl;
                }
            }
            break;
            case 3:
            {
                string name,source,destination;
                int id,type;
                cout<<"Enter passenger name : ";
                cin.ignore();
                getline(cin,name);
                cout<<"Enter passenger ID : ";
                cin>>id;
                cout<<"Enter passenger type : "<<endl;
                cout<<"1. Regular Passenger"<<endl;
                cout<<"2. Student Passenger"<<endl;
                cout<<"Enter your choice : ";
                cin>>type;
                cout<<"Enter source station : ";
                cin.ignore();
                getline(cin,source);
                cout<<"Enter destination station : ";
                getline(cin,destination);
                if (type==1)
                {
                    RegularPassenger* p = new RegularPassenger(name,id);
                    m.regular_tkt(p,source,destination);
                }
                else if (type==2)
                {
                    StudentPassenger* p = new StudentPassenger(name,id);
                    m.student_tkt(p,source,destination);
                }
                else
                    cout<<"Invalid passenger type !"<<endl;     
            }
            break;
            case 4:
            {
                m.addpassenger();    
            }
            break;
            case 5:
            {
                m.enter_metro();
            }
            break;
            case 6:
            {
                m.displayactions();
            }
            break;  
            case 7:
            {
                cout<<"Thank you for using Metro Transit Manager!"<<endl;
            }
            break;
            default:
            {
                cout<<"Invalid choice !"<<endl;
            }
        }
    }
    while(choice!=7);
    return 0;
}
