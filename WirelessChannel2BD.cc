

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class WirelessChannel2BD : public cSimpleModule
{
    private:
        int count;
        cMessage *sinkNodeMsg;
        cMessage *sourceNodeMsg;
        cMessage *lrb;
        cMessage *srb;
        double probability;
    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
    public:
        WirelessChannel2BD();
};

Define_Module(WirelessChannel2BD);
WirelessChannel2BD::WirelessChannel2BD() {
    sinkNodeMsg = nullptr;
    sourceNodeMsg = nullptr;
}

void WirelessChannel2BD::initialize()
{
    sinkNodeMsg =  new cMessage("sinkNode");
    sourceNodeMsg =  new cMessage("sourceNode");
    lrb = new cMessage("lrb");
    srb = new cMessage("srb");
}

void WirelessChannel2BD::handleMessage(cMessage *msg)
{
    cModule *c = getModuleByPath("two_bd");
    double cur_ds = c->par("absolute_distance");
    int range = (int)c->par("range");

    probability= cur_ds/4*100;
    double rand= uniform(0, 1);

    if (probability >= rand){


    if (abs((double)c->par("absolute_distance")) <= (int)c->par("range")) { //change the value of Probability
        EV << "Message is " << msg->getName() ;
        if (strcmp(msg->getName(), "sinkNode") == 0 || strcmp(msg->getName(), "lrb") == 0 || strcmp(msg->getName(), "srb") == 0 || strcmp(msg->getName(), "ack") == 0) {
            count++;
            send(msg, "sourceGate$o");
        }
        else if (strcmp(msg->getName(), "sourceNode") == 0) {
            send(msg, "sinkGate$o");
        }
    }

    else {
        EV<<"\The sink is out of range of the source!";
    }

   }

    else {
          EV<<"probability dropped the packet";
      }


}




