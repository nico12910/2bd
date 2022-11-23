

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class SensorNode2BD : public cSimpleModule
{
    private:
        int count;
        cMessage *sourceNodeMessage;
        cMessage *t_on_off_toggle;
        cMessage *timeOutCheck;
        cMessage *dataPacket;
        cMessage *tx_timeout_expired;

        int msg_counter;
        int radio_state;
        int t_on;
        int t_off;
        int highDutyCycle;
        int lowDutyCycle;
        int lrbRecvd;
        int srbRecvd;
        int data_pkt_count;
        int ack_recvd_count;
        int lrb_recvd_count;
        double srb_recvd_count;
        int passage_count;
        int ack_lost;
        int drop_pkt_count;
        int distinct_pkt_sent;
        double max_time;
        double sleep_time;
        double p_duration;
        double p_tx;
        double p_rx;




    protected:
        virtual void initialize() override;
        virtual void finish() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void toggle_t_on_off();
        virtual double getRandTime();
        virtual double calculate_sleep_time();
    public:
        SensorNode2BD();
};

Define_Module(SensorNode2BD);
SensorNode2BD::SensorNode2BD() {
    sourceNodeMessage = nullptr;
    t_on_off_toggle = nullptr;
    timeOutCheck = nullptr;
    dataPacket = nullptr;
    tx_timeout_expired = nullptr;
}


void SensorNode2BD::initialize()
{
        msg_counter = 0;
        radio_state = 0;
        count = 0;
        ack_lost=0;
        passage_count = 0;
        drop_pkt_count=0;

        t_on = -1234;
        t_off = -1234;
        highDutyCycle = -1234;
        lowDutyCycle = 1;
        lrbRecvd = -1234;
        srbRecvd = -1234;
        sleep_time = 0.0;
        max_time = 0.0;
        data_pkt_count = 0;
        ack_recvd_count = 0;
        lrb_recvd_count = 0;
        srb_recvd_count = 0;
        p_duration= 0.004;
        p_tx=52.2; // in mw
        p_rx=56.4;  //in mW




        cModule *c = getModuleByPath("two_bd");
        distinct_pkt_sent = c->par("distinct_pkt_sent");


        t_on_off_toggle = new cMessage("t_on_off_toggle");
        timeOutCheck = new cMessage("timeOutCheck");
        scheduleAt(simTime().dbl() + getRandTime() , t_on_off_toggle);

}

double SensorNode2BD::calculate_sleep_time() {
    double t_off_calc;
    t_off_calc = 0.0;
    double lamda;
    if (lowDutyCycle == 1) {
        lamda = (1.3/100);

        t_off_calc = ((t_on / lamda) - t_on)/1000.0;
        t_off_calc = 7;

    }
    else if (highDutyCycle == 1) {
        lamda = (3/100);

        t_off_calc = ((t_on / lamda) - t_on)/1000.0;
        t_off_calc = 3.2;
    }
    EV << "Calculated Sleep Time = " << t_off_calc ;
    return t_off_calc;

}
void SensorNode2BD::toggle_t_on_off() {
    if (t_on_off_toggle != nullptr )
        cancelAndDelete(t_on_off_toggle);
    t_on_off_toggle = new cMessage("t_on_off_toggle");
    if (t_on == -1234 && t_off == -1234) {
        //first try

        radio_state = 1;
        t_on = 1;
        t_off = 0;
        EV << "Radio is on!";
        //set t_on true and toggle it 100 ms later
        scheduleAt(simTime().dbl() + 0.1, t_on_off_toggle);
    }
    else if (t_on == 1 && t_off == 0) {
        //toggle t_on
        t_on = 0;
        t_off = 1;
        EV << "Radio is off!";
        scheduleAt(simTime().dbl() + calculate_sleep_time() , t_on_off_toggle);
    }
    else if (t_on == 0 && t_off == 1) {
        //toggle t_off
        t_on = 1;
        t_off = 0;
        //set t_on true and toggle it 100 ms later
        EV << "Radio is on!";
        scheduleAt(simTime().dbl() + 0.1, t_on_off_toggle);

    }
}

double SensorNode2BD::getRandTime() {

    double rand_time, period;
    period = 0.00032;
    int rv_int;
    rv_int = intuniform(0,1);
    rand_time = ((double)rv_int)*period;
    return rand_time;
}
void SensorNode2BD::handleMessage(cMessage *msg)
{
    cModule *c = getModuleByPath("two_bd");
    if ((int)c->par("cur_passage") == 33) {
        EV <<"I am here!";
    }

    if (strcmp(msg->getName(), "t_on_off_toggle") == 0) {
        cModule *c = getModuleByPath("two_bd");
        if ((int)c->par("cur_passage") <= (int)c->par("tot_passages")) {
            toggle_t_on_off();
        }

    }
    else if (strcmp(msg->getName(), "lrb") == 0) {
        EV << "Got LRB";
        if (t_on == 0) {
            //drop packet
        }
        else {
            EV << "LRB received";
            //switch to hdc
            if (lrbRecvd == -1234 or lrbRecvd == 0) {
                //was not in lrb discovery mode
                lrb_recvd_count++;
                highDutyCycle = 1;
                lowDutyCycle = 0;
                //monitor hdc timeout
                // calculate max time an MS can stay at disc range
                //max time = (2 * range) / (speed of MS in m/s)
                max_time = (2 * 100) / 11.1111;

            }
        }
        delete msg;
    }
    else if (strcmp(msg->getName(), "srb") == 0) {
        EV << "Got SRB";
        if (t_on == 0) {
            //drop packet
        }
        else {
            if (highDutyCycle != 1) {
                //this is weird
            } else {
                //srb received
                //initiate data transfer
                srb_recvd_count++;



                if (dataPacket != nullptr) {
                    cancelAndDelete(dataPacket);

                }
                dataPacket = new cMessage("dataPacket");
                scheduleAt(simTime() + 0.00000001, dataPacket);
            }

        }
        delete msg;
    }
    else if (strcmp(msg->getName(), "dataPacket") == 0) {
        cModule *c = getModuleByPath("two_bd");
        if ((int)c->par("cur_passage") == 21) {
            EV <<"I am here!";
        }
        if (ack_lost < 1) {
            //new packet
            cModule *c = getModuleByPath("two_bd");
            c->par("distinct_pkt_sent") = (int)c->par("distinct_pkt_sent") + 1;
        }
        //stop timer event

        double data_timeout = 2 * 0.01 + 0.04 + 0.04;

        cMessage *data_pkt = new cMessage("sourceNode");
        send(data_pkt, "gate$o");

        data_pkt_count++;

    }
//    else if (strcmp(msg->getName(), "tx_timeout_expired") == 0) {
//        ack_lost++;
//
//        if (ack_lost < 3) {
//            //retransmit
//            if (dataPacket != nullptr) {
//                cancelAndDelete(dataPacket);
//
//            }
//            dataPacket = new cMessage("dataPacket");
//            scheduleAt(simTime() + 0.00000001, dataPacket);
//            data_pkt_count++;
//        }
//        else {
//            //drop packet
//            //ms out of range
//            ack_lost = 0;
//            lowDutyCycle = 1;
//            highDutyCycle = 0;
//            t_on = 1;
//            t_off = 0;
//            drop_pkt_count++;
//            scheduleAt(simTime() + 0.00000001, t_on_off_toggle);
//            if (dataPacket != nullptr)
//                cancelAndDelete(dataPacket);
//            if (tx_timeout_expired != nullptr)
//                cancelAndDelete(tx_timeout_expired);
//        }
//    }
    else if (strcmp(msg->getName(), "timeOutCheck") == 0) {
        //timeout noticed
        //change hdc to ldc
        highDutyCycle = 0;
        lowDutyCycle = 1;
    }
    else if (strcmp(msg->getName(), "ack") == 0) {
        ack_recvd_count++;
        msg_counter++;
        //
        ack_lost = 0;


        cModule *c = getModuleByPath("two_bd");
        if ((int)c->par("cur_passage") == (int)c->par("tot_passages")) {
                EV << "\nSensorNode: LRB Received " << lrb_recvd_count;
               EV << "\nSensorNode: SRB Received " << srb_recvd_count;
               EV << "\nSensorNode: Data Sent " << data_pkt_count;
               EV << "\nSensorNode: Ack Received " << ack_recvd_count;
        }



        if (msg_counter >0 && msg_counter < 95) {
                    if (dataPacket != nullptr) {
                        cancelAndDelete(dataPacket);

                    }
            dataPacket = new cMessage("dataPacket");
            scheduleAt(simTime() + 0.00000001, dataPacket);

        }
        else {
            msg_counter = 0;
            //drop packet
            //ms out of range
            ack_lost = 0;
            lowDutyCycle = 1;
            highDutyCycle = 0;
            t_on = 1;
            t_off = 0;
//            drop_pkt_count++;


            if (t_on_off_toggle != nullptr)
                cancelAndDelete(t_on_off_toggle);
            t_on_off_toggle = new cMessage("t_on_off_toggle");
            scheduleAt(simTime() + 0.00000001, t_on_off_toggle);
        }
    }


}
void SensorNode2BD::finish() {

    cModule *c = getModuleByPath("two_bd");

    EV << "\nSensorNode: LRB Received " << lrb_recvd_count;
    EV << "\nSensorNode: SRB Received " << srb_recvd_count;

    EV << "\nSensorNode: Number of Mobile Sink Discovered " << srb_recvd_count;



    double discovery_ratio= (srb_recvd_count/(int)c->par("tot_passages"))*100;

    double average_throughput= (data_pkt_count*133)/(int)c->par("tot_passages");

    double E_DP= (lrb_recvd_count/1000.0) * p_duration* p_rx;

    double E_DTP = ((lrb_recvd_count + srb_recvd_count)/1000.0)*p_duration*p_rx+ (ack_recvd_count/1000.0)*p_duration*p_rx;


    EV << "\nDiscovery ratio " << discovery_ratio;

    EV << "\naverage_throughput " << average_throughput;

    EV << "\nE_DP " << E_DP;

    EV << "\nE_DTP " << E_DTP;


}




