#include <iostream>
#include <limits.h>

using namespace std;
 
class TCPConnection {
private:
    int cwnd;
    int ssthresh;
    int rtt;

public:
    TCPConnection(int cwnd_ = 1, int ssthresh_ = 65535, int rtt_ = 0) : cwnd(cwnd_), ssthresh(ssthresh_), rtt(rtt_) {}
    
    int getCwnd() { return cwnd; }
    int getSsthresh() { return ssthresh; }
    int getRtt() { return rtt; }

    void setCwnd(int new_cwnd) { cwnd = new_cwnd; }
    void setSsthresh(int new_ssthresh) { ssthresh = new_ssthresh; }
    void setRtt(int new_rtt) { rtt = new_rtt; }

    void print(){
        std::cout << "cwnd = " << getCwnd() << ", ssthresh = " << getSsthresh() << std::endl;
    }
};