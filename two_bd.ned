//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

network two_bd {
    parameters:
        
        int distinct_pkt_sent=0;
        int distinct_ack_received=0;
        int range;
        int tot_passages;
        int cur_passage=0;
        
        
        double absolute_distance=102;
        double x_cordinate=-102;
        double y_cordinate=5;


    submodules:
        SensorNode: SensorNode2BD;
        MobileSinkNode: MobileSinkNode2BD;
        WirelessChannel: WirelessChannel2BD;
    connections:
          MobileSinkNode.gate <-->  WirelessChannel.sinkGate;
          WirelessChannel.sourceGate <--> SensorNode.gate;

}

simple SensorNode2BD
{
    parameters:

        @display("p=0,0");
    gates:

          inout gate;
}

simple MobileSinkNode2BD
{
    parameters:        
        @display("p=$x_cordinate,$y_cordinate");
    gates:

		  inout gate;
}


simple WirelessChannel2BD
{
    parameters:

        @display("p=15,10");
    gates:

          
		  inout sourceGate;
          inout sinkGate;
          
}
  
