
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class MobileSinkNode2BD : public cSimpleModule
{
    protected:
        virtual void initialize() override;
        virtual void finish() override;
        virtual void handleMessage(cMessage *msg) override;

        virtual double calculate_drift();
        virtual double getRandTime();
    private:
        cMessage *initial_msg;
        cMessage *update_msg;
        cMessage *beacon_msg;
        cMessage *ack_msg;
        int tot_passages;
        double x_cordinate;
        double y_cordinate;
        double update_time;
        double diff, dist;
        int distinct_rx_count;
        int end_sim;
        int range;
        int comm_range;
        int previous_beacon;
        int lrb_sent_count;
        int srb_sent_count;
        int data_recv_count;
        int ack_sent_count;
        int passage_count;
        int cur_passage;
        int distinct_ack_received;
        int correctly_rx;
        bool ms_in_range;
        double speed;
        double beacon_interval;
        double absolute_distance;
    public:
        MobileSinkNode2BD();
};

Define_Module(MobileSinkNode2BD);

MobileSinkNode2BD::MobileSinkNode2BD()
{
    initial_msg = nullptr;
    update_msg = nullptr;
    beacon_msg = nullptr;
    ack_msg = nullptr;
}

double MobileSinkNode2BD::calculate_drift() {
    x_cordinate = x_cordinate + 0.0111;
    getDisplayString().setTagArg("p",0,x_cordinate);
    getDisplayString().setTagArg("p",1,y_cordinate);
    return x_cordinate;
}

double MobileSinkNode2BD::getRandTime() {

    double rand_time, period;
    period = 0.00032;
    int rv_int;
    rv_int = intuniform(0,1);
    rand_time = ((double)rv_int)*period;
    return rand_time;
}

void MobileSinkNode2BD::initialize()
{
    cModule *c = getModuleByPath("two_bd");
    end_sim = 0;
    passage_count = 0;
    distinct_rx_count=-1;
    correctly_rx=0;
    lrb_sent_count = 0;
    srb_sent_count = 0;
    data_recv_count = 0;
    ack_sent_count = 0;
    x_cordinate = -102;
    y_cordinate = 5;
    ms_in_range = false;
    beacon_interval = 0.2;
    range = c->par("range");
    comm_range = 50;
    distinct_ack_received = c->par("distinct_ack_received");



    tot_passages = c->par("tot_passages");

    speed = 0.01; // meter/ms

    previous_beacon = -1234;
    int module_no = getIndex();
    EV << "Sink Module no is" << module_no;
    update_msg = new cMessage("update_distance");
    beacon_msg = new cMessage("beacon");
    ack_msg = new cMessage("ack");
    scheduleAt(simTime() + 0.000000001, update_msg);

}

void MobileSinkNode2BD::handleMessage(cMessage *msg)
{
    if (end_sim == 1) {

        finish();
    }


    if (msg == update_msg) {
         dist = calculate_drift();
         dist = abs(dist);
         diff = sqrt( pow(abs(0 - dist), 2) + pow(5, 2) );

         diff = abs(diff);
         cModule *c = getModuleByPath("two_bd");


         EV << "Distance update == " << diff;
         update_time = 0.001;
         diff = abs(dist);
         c->par("absolute_distance") = diff;
         if (diff < range) {

             ms_in_range = true;
             if (previous_beacon == -1234) {
                 if (beacon_msg != nullptr)
                     cancelAndDelete(beacon_msg);
                 if (passage_count <= tot_passages) {
                     beacon_msg = new cMessage("beacon");
                     scheduleAt(simTime() + 0.00000001, beacon_msg);
                 }
             }

         }
         else {
             if (ms_in_range) {
                 ms_in_range = false;
                 x_cordinate = -102;
                 y_cordinate = 5;
                 update_time = getRandTime();
                 passage_count++;
                 cModule *c = getModuleByPath("two_bd");
                 c->par("cur_passage") = passage_count;
                 if (passage_count == 1000) {
                     c->par("cur_passage") = 1000;

                     EV << "\nPassage: " << passage_count;
                     EV << "\nSinkNode: LRB Sent " << lrb_sent_count;
                     EV << "\nSinkNode: SRB Sent " << srb_sent_count;
                     EV << "\nSinkNode: Data Received " << data_recv_count;
                     EV << "\nSinkNode: Ack Sent " << ack_sent_count;
                     end_sim = 1;
                 }

             }

         }
         if (update_msg != nullptr)
             cancelAndDelete(update_msg);
         if (passage_count <= tot_passages) {
             update_msg = new cMessage("update_distance");
             scheduleAt(simTime() + update_time, update_msg);
         }

    }
    else if (msg == beacon_msg) {
        if (previous_beacon == -1234) {
            //first beacon
            //send immediately
            //check range for lrb
            if (diff <= range)
            {
                EV << "Sending First LRB";
                cMessage *lrb = new cMessage("lrb");
                send(lrb, "gate$o");
                previous_beacon = 1;
                lrb_sent_count++;

            }

        }
        else {
            if (previous_beacon == 0) {
                //lrb
                if (diff <= range){
                    EV << "Sending LRB";
                    cMessage *lrb = new cMessage("lrb");
                    send(lrb, "gate$o");
                    previous_beacon = 1;
                    lrb_sent_count++;
                }

            }
            else if (previous_beacon == 1) {
                //short beacon to be sent
                if (diff <= range){
                    EV << "Sending SRB";
                    cMessage *srb = new cMessage("srb");
                    send(srb, "gate$o");
                    previous_beacon = 0;
                    srb_sent_count++;
                }
            }

        }
        if (beacon_msg != nullptr)
            cancelAndDelete(beacon_msg);
        if (passage_count <= tot_passages) {
            beacon_msg = new cMessage("beacon");
            scheduleAt(simTime() + beacon_interval, beacon_msg);
        }
    }
    else {
        EV << "Data pkt received";
        EV << "\n(std::string) msg->getName() == " << (std::string) msg->getName() ;

        data_recv_count++;



        if (ack_msg != nullptr)
            cancelAndDelete(ack_msg);
        ack_msg = new cMessage("ack");
        send(ack_msg, "gate$o");
        ack_sent_count++;
    }

}
void MobileSinkNode2BD::finish() {
    EV << "\nPassage: " << passage_count;
    EV << "\nSinkNode: LRB Sent " << lrb_sent_count;
    EV << "\nSinkNode: SRB Sent " << srb_sent_count;
    EV << "\nSinkNode: Data Received " << data_recv_count;
    EV << "\nSinkNode: Ack Sent " << ack_sent_count;
    end_sim = 1;

}





