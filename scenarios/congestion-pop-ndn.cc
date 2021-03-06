/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "congestion-pop-experiment.h"
#include <ns3/core-module.h>

#include <ns3/ndnSIM-module.h>

// for ndn::AppDelayTracer
#include <ns3/ndnSIM/utils/tracers/ndn-app-delay-tracer.h>

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

NS_LOG_COMPONENT_DEFINE ("Experiment");

#define _LOG_INFO(x) NS_LOG_INFO(x)

int
main (int argc, char *argv[])
{
  _LOG_INFO ("Begin congestion-pop scenario (NDN)");

  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("60"));

  Config::SetDefault ("ns3::RttEstimator::MaxRTO", StringValue ("1s"));

  Config::SetDefault ("ns3::ndn::ConsumerWindow::Window", StringValue ("4"));
  Config::SetDefault ("ns3::ndn::ConsumerWindow::InitialWindowOnTimeout", StringValue ("false"));

  uint32_t run = 1;
  CommandLine cmd;
  cmd.AddValue ("run", "Simulation run", run);
  cmd.Parse (argc, argv);

  Config::SetGlobal ("RngRun", IntegerValue (run));
  NS_LOG_INFO ("seed = " << SeedManager::GetSeed () << ", run = " << SeedManager::GetRun ());

  CongestionPopExperiment experiment;
  NS_LOG_INFO ("Run " << run);
  string prefix = "results/congestion-pop-run-" + lexical_cast<string> (run) + "-ndn-";

  experiment.GenerateRandomPairs (20);
  experiment.DumpPairs (prefix + "apps.log");

  experiment.ConfigureTopology ("topologies/sprint-pops");
  experiment.InstallNdnStack ();
  ApplicationContainer apps = experiment.AddNdnApplications ();

  for (uint32_t i = 0; i < apps.GetN () / 2; i++)
    {
      apps.Get (i*2)->SetStartTime (Seconds (1+i));
      apps.Get (i*2 + 1)->SetStartTime (Seconds (1+i));
    }

  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::AppDelayTracer> > >
    tracers = ndn::AppDelayTracer::InstallAll (prefix + "consumers-seqs.log");

  // CcnxTraceHelper traceHelper;
  // traceHelper.EnableSeqsAppAll ("ns3::CcnxConsumerWindow", prefix + "consumers-seqs.log");
  // traceHelper.EnableWindowsAll (prefix + "windows.log");

  // config.ConfigureAttributes ();
  experiment.Run (Seconds (200.0));

  _LOG_INFO ("Finish congestion-pop scenario (NDN)");
  return 0;
}
