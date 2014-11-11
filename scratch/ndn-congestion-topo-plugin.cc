// ndn-congestion-topo-plugin.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 25);
  topologyReader.SetFileName ("src/ndnSIM/examples/topologies/topo-6-node.txt");
  topologyReader.Read ();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru",
                              "MaxSize", "10000");
  ndnHelper.InstallAll ();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Getting containers for the consumer/producer
  Ptr<Node> consumer1 = Names::Find<Node> ("Src1");
  Ptr<Node> consumer2 = Names::Find<Node> ("Src2");

  Ptr<Node> producer1 = Names::Find<Node> ("Dst1");
  Ptr<Node> producer2 = Names::Find<Node> ("Dst2");

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute ("Frequency", StringValue ("100")); // 100 interests a second

  // on the first consumer node install a Consumer application
  // that will express interests in /dst1 namespace
  consumerHelper.SetPrefix ("/dst1");
  consumerHelper.Install (consumer1);

  // on the second consumer node install a Consumer application
  // that will express interests in /dst2 namespace
  consumerHelper.SetPrefix ("/dst2");
  consumerHelper.Install (consumer2);
  
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));  

  // Register /dst1 prefix with global routing controller and
  // install producer that will satisfy Interests in /dst1 namespace
  ndnGlobalRoutingHelper.AddOrigins ("/dst1", producer1);
  producerHelper.SetPrefix ("/dst1");
  producerHelper.Install (producer1);

  // Register /dst2 prefix with global routing controller and
  // install producer that will satisfy Interests in /dst2 namespace
  ndnGlobalRoutingHelper.AddOrigins ("/dst2", producer2);
  producerHelper.SetPrefix ("/dst2");
  producerHelper.Install (producer2);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes ();

  Simulator::Stop (Seconds (20.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
