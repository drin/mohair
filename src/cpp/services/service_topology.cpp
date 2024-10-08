// ------------------------------
// License
//
// Copyright 2024 Aldrin Montana
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// ------------------------------
// Dependencies

#include "services/service_topology.hpp"
#include "services/client_mohair.hpp"


// ------------------------------
// Functions and Callables

namespace mohair::services {

  // ----------
  // Simple struct to make it easier to modularize config parsing

  struct TopologyConfigEntry {
    size_t entry_ndx       { 0 };
    size_t loc_startndx    { 0 };
    size_t loc_len         { 0 };
    size_t count_entrylocs { 0 };
  };


  // ----------
  // Convenience functions

  // General support of TopologyService
  vector<ActionType> SupportedActionsForTopology() {
    return {
       { ActionActivate  , "Add a service to the CS system"      }
      ,{ ActionDeactivate, "Remove a service from the CS system" }
    };
  }


  // Support for configuration parsing
  unique_ptr<ServiceConfig> AddDownstream( unique_ptr<ServiceConfig>& upstream
                                          ,Location&                  down_loc) {
    unique_ptr<ServiceConfig> new_srv { upstream->add_downstream_services() };
    new_srv->set_service_location(down_loc.ToString());

    return new_srv;
  }


  Result<Location> LocationFromEntry(string& src, TopologyConfigEntry& entry_info) {
    string loc_uri { &(src[entry_info.loc_startndx]), entry_info.loc_len };
    return Location::Parse(loc_uri);
  }


  Result<unique_ptr<ServiceHierarchy>>
  TopologyFromConfig(const char* config_fpath, bool be_verbose) {
    // Read data from the file
    string topo_config;
    if (not FileToString(config_fpath, topo_config)) {
      return Status::Invalid("Failed to read data from topology config");
    }

    // Parse the data
    // unordered_map<Location, unique_ptr<ServiceConfig>>
    auto service_map = std::make_unique<ServiceHierarchy>();
    TopologyConfigEntry entry_info;

    Location                  service_loc;
    unique_ptr<ServiceConfig> parsed_service { nullptr };

    auto prev_sym = '\0';
    for (size_t char_ndx = 0; char_ndx < topo_config.length(); ++char_ndx) {
      auto curr_sym = topo_config[char_ndx];
      if (char_ndx > 0) { prev_sym = topo_config[char_ndx - 1]; }

      // Skip an empty line
      if (curr_sym == '\n' and prev_sym == '\n') {
        entry_info.loc_startndx = 1 + char_ndx;
        continue;
      }

      // If the entry starts with a '#' skip everything until a newline (comment)
      else if (curr_sym == '#'  and entry_info.entry_ndx == 0) {
        do { ++char_ndx; } while (topo_config[char_ndx] != '\n');
      }

      // Top-level entries start with `->`
      // otherwise, `->` separates upstream and downstream locations
      if (curr_sym == '-' and entry_info.entry_ndx > 0) {
        // Otherwise, upstream locations and downstream locations are separated by `->`
        entry_info.loc_len = char_ndx - entry_info.loc_startndx;
        ARROW_ASSIGN_OR_RAISE(service_loc, LocationFromEntry(topo_config, entry_info));

        // clear state for next location
        entry_info.loc_startndx = 0;
        entry_info.loc_len      = 0;

        // Set the active service for the entry
        if (service_map->cs_devices.find(service_loc) == service_map->cs_devices.end()) {
          parsed_service = std::make_unique<ServiceConfig>();
          parsed_service->set_service_location(service_loc.ToString());
        }
        else {
          parsed_service = std::move(service_map->cs_devices[service_loc]);
        }
      }

      else if (curr_sym == '>' && prev_sym == '-') {
        entry_info.loc_startndx = 1 + char_ndx;
      }

      else if (curr_sym == ',') {
        if (parsed_service == nullptr) {
          return Status::Invalid("Malformed config: expected upstream location first");
        }

        // Parse the location
        entry_info.loc_len = char_ndx - entry_info.loc_startndx;
        ARROW_ASSIGN_OR_RAISE(
           Location downstream_loc
          ,LocationFromEntry(topo_config, entry_info)
        );

        // Add the downstream config
        service_map->upstream_locs[downstream_loc] = service_loc;
        service_map->cs_devices[downstream_loc]    = (
          AddDownstream(parsed_service, downstream_loc)
        );

        // Prep for next location
        entry_info.loc_len      = 0;
        entry_info.loc_startndx = 1 + char_ndx;
      }

      // We have completed a top-level Location or a ServiceConfig
      else if (topo_config[char_ndx] == '\n') {
        // Parse the location
        entry_info.loc_len = char_ndx - entry_info.loc_startndx;
        ARROW_ASSIGN_OR_RAISE(
           Location downstream_loc
          ,LocationFromEntry(topo_config, entry_info)
        );

        // Make sure the ServiceConfig doesn't yet exist
        if (service_map->cs_devices.find(downstream_loc) != service_map->cs_devices.end()) {
          MohairDebugMsg("[Error] Parsed duplicate location");
          return Status::Invalid("Downstream location already exists");
        }

        // Add it to the list of top-level locations if there's no active ServiceConfig
        else if (parsed_service == nullptr) {
          if (be_verbose) { MohairDebugMsg("Parsed top-level entry"); }
          service_map->cs_servers.push_back(downstream_loc);
        }

        // Otherwise, create it then add to the service map
        else {
          if (be_verbose) {
            MohairDebugMsg("Parsed entry for [" << service_loc.ToString() << "]");
          }

          service_map->upstream_locs[downstream_loc] = service_loc;
          service_map->cs_devices[downstream_loc]    = (
            AddDownstream(parsed_service, downstream_loc)
          );

          service_map->cs_devices[service_loc] = std::move(parsed_service);
        }

        // Add completed entry to service map and clear state for next entry
        parsed_service.reset();
        entry_info.entry_ndx       = 0;
        entry_info.loc_startndx    = 1 + char_ndx;
        entry_info.loc_len         = 0;
        entry_info.count_entrylocs = 0;

        continue;
      }

      ++entry_info.entry_ndx;
    }

    return service_map;
  }

  void StringifyServiceInfo(stringstream& sstream, string prefix, ServiceConfig& srv_info) {
    sstream << prefix << srv_info.service_location() << " >>\t";
    
    auto downstream_srvs = srv_info.downstream_services();
    for (int srv_ndx = 0; srv_ndx < downstream_srvs.size(); ++srv_ndx) {
      auto& downstream_srv = downstream_srvs[srv_ndx];

      if (srv_ndx > 0) { sstream << ", "; }
      sstream << downstream_srv.service_location();
    }

    string downstream_prefix { prefix + "\t" };
    for (int srv_ndx = 0; srv_ndx < downstream_srvs.size(); ++srv_ndx) {
      sstream << std::endl;
      StringifyServiceInfo(sstream, downstream_prefix, downstream_srvs[srv_ndx]);
    }

    sstream << std::endl;
  }

  void PrintTopology(ServiceHierarchy* service_map) {
    stringstream print_stream;
    string       empty_prefix;

    vector<Location>& tl_servers = service_map->cs_servers;
    service_topology& dev_map    = service_map->cs_devices;

    // Print all top-level locations
    for (size_t server_ndx = 0; server_ndx < tl_servers.size(); ++server_ndx) {
      print_stream << tl_servers[server_ndx].ToString() << std::endl;
    }
    print_stream << std::endl;

    // Then print the hierarchy
    for (size_t server_ndx = 0; server_ndx < tl_servers.size(); ++server_ndx) {
      // srv_info is pair<Location, unique_ptr<ServiceConfig>>
      const auto& srv_info = dev_map.find(tl_servers[server_ndx]);

      if (srv_info != dev_map.end()) {
        StringifyServiceInfo(print_stream, empty_prefix, *(srv_info->second));
      }
    }

    std::cout << print_stream.str() << std::endl;
  }

} // namespace: mohair::services


// ------------------------------
// Classes

// >> Hash functor implementations
namespace mohair::services {

  std::size_t HashFunctorMohairTicket::operator()(const Ticket& mohair_ticket) const {
    // A mohair ticket may be a service location or a query identifier
    static std::hash<string> HashMohairTicketId;

    return HashMohairTicketId(mohair_ticket.ticket);
  }

  std::size_t HashFunctorMohairLocation::operator()(const Location& mohair_location) const {
    // A location is essentially a URI
    static std::hash<string> HashMohairLocation;

    return HashMohairLocation(mohair_location.ToString());
  }

} // namespace: mohair::services


// >> TopologyService implementations
namespace mohair::services {

  // |> Helper functions
  Result<FlightEndpoint>
  TopologyService::GetDownstreamServices([[maybe_unused]] FlightEndpoint& upstream_srv) {
    return Status::NotImplemented("WIP");
  }

  // >> Custom Flight API
  Status
  TopologyService::DoActivateService( [[maybe_unused]] const ServerCallContext&  context
                                     ,                 const shared_ptr<Buffer>  serialized_loc
                                     ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
    MohairDebugMsg("Handling request: [register-service]");

    // Deserialize the location URI and parse it into a `Location`
    string location_uri = serialized_loc->ToString();
    ARROW_ASSIGN_OR_RAISE(auto service_loc, Location::Parse(location_uri));
    
    // Verify the location exists and is inactive (not yet claimed by another device)
    const auto& config_entry = service_map->cs_devices.find(service_loc);
    if (config_entry == service_map->cs_devices.end()) {
      std::cerr << "Error: Request to register invalid location" << std::endl;
      return Status::Invalid("Location not part of configuration");
    }
    else if (config_entry->second->is_active()) {
      std::cerr << "Error: Request to double register location" << std::endl;
      return Status::Invalid("Location already active");
    }

    // Activate the configuration and send it in the response
    MohairDebugMsg("Registering location [" << service_loc.ToString() << "]");
    config_entry->second->set_is_active(true);

    string response_payload;
    auto&  service_cfg = config_entry->second;
    if (not service_cfg->SerializeToString(&response_payload)) {
      return Status::Invalid("Unable to serialize config for response");
    }

    // Create a response stream with a single buffer containing the payload
    (*response_stream) = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { FlightResult { Buffer::FromString(response_payload) } }
    );

    // If there is an upstream service, send it a view change
    const auto& upstream_entry = service_map->upstream_locs.find(service_loc);
    if (upstream_entry != service_map->upstream_locs.end()) {
      Location& upstream_loc = upstream_entry->second;
      auto&     upstream_cfg = service_map->cs_devices[upstream_loc];

      MohairDebugMsg("Connecting to service [" << upstream_loc.ToString() << "]");
      auto mohair_conn = MohairClient::ForLocation(upstream_loc);
      if (mohair_conn == nullptr) {
        return Status::Invalid("Unable to connect to service");
      }

      MohairDebugMsg("Sending view change");
      mohair_conn->SendViewUpdate(*upstream_cfg);
    }

    return Status::OK();
  }

  Status
  TopologyService::DoDeactivateService( [[maybe_unused]] const ServerCallContext&  context
                                       ,                 const shared_ptr<Buffer>  serialized_loc
                                       ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
    MohairDebugMsg("Handling request: [" << ActionDeactivate << "]");

    // Deserialize location URI and parse it into a `Location`
    string location_uri = serialized_loc->ToString();
    ARROW_ASSIGN_OR_RAISE(auto service_loc, Location::Parse(location_uri));
    
    // Verify we are de-registering a previously registered location
    const auto& config_entry = service_map->cs_devices.find(service_loc);
    if (config_entry == service_map->cs_devices.end()) {
      std::cerr << "Error: Request to de-register an unknown location" << std::endl;
      return Status::Invalid("Location was never registered");
    }
    else if (not config_entry->second->is_active()) {
      std::cerr << "Error: Location is already de-activated" << std::endl;
      return Status::Invalid("Location already inactive");
    }

    MohairDebugMsg("De-activating location [" << service_loc.ToString() << "]");
    config_entry->second->set_is_active(false);

    // If there is an upstream service, send it a view change
    const auto& upstream_entry = service_map->upstream_locs.find(service_loc);
    if (upstream_entry != service_map->upstream_locs.end()) {
      Location& upstream_loc = upstream_entry->second;
      auto&     upstream_cfg = service_map->cs_devices[upstream_loc];

      auto client_conn = MohairClient::ForLocation(upstream_loc);
      if (client_conn == nullptr) {
        return Status::Invalid("Unable to connect to service");
      }

      client_conn->SendViewUpdate(*upstream_cfg);
      MohairDebugMsg("Sent view change to [" << upstream_loc.ToString() << "]");
    }

    return Status::OK();
  }

  Status
  TopologyService::DoServiceAction( const ServerCallContext&  context
                                   ,const Action&             action
                                   ,unique_ptr<ResultStream>* result) {

    // known actions (uses macros from apidep_flight.hpp)
    if (action.type == ActionActivate) {
      return DoActivateService(context, action.body, result);
    }

    else if (action.type == ActionDeactivate) {
      return DoDeactivateService(context, action.body, result);
    }

    else if (action.type == ActionShutdown) {
      return DoShutdown(context);
    }

    // Catch all that returns Status::NotImplemented()
    return DoUnknown(context, action.type);
  }


  // |> Standard Flight API
  Status
  TopologyService::ListActions( [[maybe_unused]] const ServerCallContext& context
                               ,                 vector<ActionType>*      actions) {
    *actions = SupportedActionsForTopology();
    return Status::OK();
  }

} // namespace: mohair::services
