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

class TCPReno : public TCPConnection {
public:
    TCPReno(int cwnd_ = 1, int ssthresh_ = 65535, int rtt_ = 0) : TCPConnection(cwnd_, ssthresh_, rtt_) {}

    void sendData() {
        int cwnd = getCwnd();
        int ssthresh = getSsthresh();

        std::cout << "Reno Sending data with cwnd = " << cwnd << std::endl;

        if (cwnd < ssthresh) {
            // Slow Start phase
            setCwnd(cwnd * 2);
        } else {
            // Congestion Avoidance phase
            setCwnd(cwnd + 1);
        }
    }

    void onPacketLoss() {
        int cwnd = getCwnd();

        std::cout << "Reno Packet loss detected, adjusting cwnd and ssthresh" << std::endl;

        // Fast Recovery phase
        setSsthresh(cwnd / 2);
        setCwnd(getSsthresh() + 3);
    }

    void onRTTUpdate(int newRTT) {
        std::cout << "Reno Updating RTT to " << newRTT << " ms" << std::endl;
        setRtt(newRTT);
    }
};

class TCPNewReno : public TCPConnection {
private:
    bool recovery;
    int highData;
    int highACK;

public:
    TCPNewReno(int cwnd_ = 1, int ssthresh_ = 65535, int rtt_ = 0, int highData_ = 0, int highACK_ = 0)
    : TCPConnection(cwnd_, ssthresh_, rtt_), recovery(false), highData(highData_), highACK(highACK_) {}

    void sendData() {
        int cwnd = getCwnd();
        int ssthresh = getSsthresh();

        std::cout << "NewReno Sending data with cwnd = " << cwnd << std::endl;

        if (cwnd < ssthresh) {
            // Slow Start phase
           setCwnd(cwnd * 2);
        } else {
            // Congestion Avoidance phase
            setCwnd(cwnd + 1);
        }
    }

    void onPacketLoss() {
        int cwnd = getCwnd();

        std::cout << "NewReno Packet loss detected, adjusting cwnd and ssthresh" << std::endl;

        // Fast Recovery phase
        setSsthresh(cwnd / 2);
        setCwnd(getSsthresh() + 3);
        recovery = true;
        highData = cwnd;
    }

    void onRTTUpdate(int newRTT) {
        std::cout << "NewReno Updating RTT to " << newRTT << " ms" << std::endl;
        setRtt(newRTT);
    }

    void onSelectiveAck(int ackPacketNumber) {
        std::cout << "NewReno Selective Acknowledgment received for packet " << ackPacketNumber << std::endl;

        // Partial ACK handling
        if (recovery) {
            // Packet is acknowledged in SACK, move highACK pointer
            if (ackPacketNumber > highACK) {
                highACK = ackPacketNumber;
            }

            // Reno's fast recovery constrained to first ACK in the recovery window
            if (ackPacketNumber < highData) {
                int cwnd = getCwnd();
                setCwnd(cwnd + 1);
            } else {
                // End of fast recovery
                int cwnd = getCwnd();
                int ssthresh = getSsthresh();
                setCwnd(std::min(cwnd, ssthresh));
                recovery = false;
            }
        }
    }
};

class BBR : public TCPConnection {
private:
    int max_bandwidth;
    int min_rtt;

public:
    BBR(int cwnd_ = 1, int ssthresh_ = 65535, int rtt_ = 0)
    : TCPConnection(cwnd_, ssthresh_, rtt_), max_bandwidth(ssthresh_), min_rtt(INT_MAX) {}

    void updateMaxBandwidth(int bandwidth) {
        if (bandwidth > max_bandwidth) {
            max_bandwidth = bandwidth;
        }
    }

    void updateMinRtt(int rtt) {
        if (rtt < min_rtt) {
            min_rtt = rtt;
        }
    }

    void sendData() {
        int cwnd = getCwnd();
        int ssthresh = getSsthresh();

        std::cout << "BBR Sending data with cwnd = " << cwnd << std::endl;

        // BBR sends data at the rate of max_bandwidth and increases the rate of sendData based on the min_rtt.
        cwnd = std::min(cwnd * 2, max_bandwidth * min_rtt);

        setCwnd(cwnd);
    }

    void onPacketLoss() {

        std::cout << "BBR Packet loss detected, adjusting max_bandwidth and min_rtt" << std::endl;

        // The handling of packet loss in BBR is updating max_bandwidth and min_rtt, rather than adjusting cwnd and ssthresh immediately.
        // It continues monitoring the network conditions for longer term than Reno or NewReno.
        updateMaxBandwidth(getCwnd() / getRtt());
        updateMinRtt(getRtt());
    }

    void onRTTUpdate(int newRTT) {
        setRtt(newRTT);
        if (newRTT < min_rtt) {
            min_rtt = newRTT;
        }
    }
};

int main() {
    // Instantiate class objects with different initial cwnd and ssthresh values
    // Simulate packet loss rates and RTTs
    int packetLossRates[] = {1, 5, 10}; // percentages
    int RTTs[] = {50, 100, 200}; // milliseconds
    for (auto& RTT : RTTs) {
        TCPReno reno(10, 100);
        TCPNewReno newreno(10, 100);
        BBR bbr(10, 100);
        // Use 'RTT' inside this block
        std::cout << "RTT is " << RTT << " ms" << std::endl;
        reno.onRTTUpdate(RTT);
        newreno.onRTTUpdate(RTT);
        bbr.onRTTUpdate(RTT);
        for (int i = 1; i < 20; i++) {
            if (i % 10 == 0) {
                reno.onPacketLoss();
                newreno.onPacketLoss();
                bbr.onPacketLoss();
            } else {
                reno.sendData();
                newreno.sendData();
                bbr.sendData();
            }
        }        
        cout << "------------------" << endl;
        cout << "Reno: ";
        reno.print();
        cout << "NewReno: ";
        newreno.print();
        cout << "BBR: ";
        bbr.print();
        cout << "==========================" << endl;
    }
    return 0;
}
