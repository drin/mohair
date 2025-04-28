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

#include <cctype>

#include "services/service_topology.hpp"
#include "services/client_skytether.hpp"


// ------------------------------
// Variables

namespace skytether::services {

  const string empty_prefix   { ""   };
  const string base_prefix    { "  " };
  const string link_symbol    { "─"  };
  const string nonleaf_prefix { "├"  };
  const string    leaf_prefix { "└"  };

  //! A function that returns a vector of actions the topology service supports
  vector<ActionType> SupportedActionsForTopology() {
    return {
       { ActionActivate  , "Add a service to the CS system"      }
      ,{ ActionDeactivate, "Remove a service from the CS system" }
    };
  }

} // namespace: skytether::services


// ------------------------------
// Parsing Functions

namespace skytether::services {

  // >> Function signatures
  string PeekLine(string& config_body, size_t& char_ndx);
  string PeekEntryKey(string& config_body, size_t& char_ndx);
  string ParseLine(string& config_body, size_t& char_ndx);


  // >> Generalized parsing functions
  // Functions that check the character for specific values
  bool IsSpace(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == ' ';
  }

  bool IsLineEnd(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == '\n';
  }

  bool IsWhitespace(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and std::isspace(config_body[char_ndx]);
  }

  bool IsEntrySeparator(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == '=';
  }

  bool IsListSeparator(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == ',';
  }

  bool IsStringMarker(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == '\'';
  }

  bool IsHeaderOpen(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == '[';
  }

  bool IsHeaderClose(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == ']';
  }

  bool IsListOpen(string& config_body, size_t& char_ndx) {
    return IsHeaderOpen(config_body, char_ndx);
  }

  bool IsListClose(string& config_body, size_t& char_ndx) {
    return IsHeaderClose(config_body, char_ndx);
  }

  bool IsKeyCharacter(string& config_body, size_t& char_ndx) {
    if (char_ndx >= config_body.length()) { return false; }

    auto entry_char { static_cast<unsigned char>(config_body[char_ndx]) };
    return std::isalnum(entry_char) or entry_char == '-' or entry_char == '_';
  }

  bool IsSectionHeader(string& config_body, size_t& char_ndx) {
    return char_ndx < config_body.length() and config_body[char_ndx] == '[';
  }

  bool IsListMarker(string& config_body, size_t& char_ndx) {
    return IsSectionHeader(config_body, char_ndx);
  }

  bool IsEntryDownstream(string& config_body, size_t& char_ndx) {
    return PeekEntryKey(config_body, char_ndx) == "downstream";
  }

  // Functions that move a shared position forward
  void ConsumeSpaces(string& config_body, size_t& char_ndx) {
    while (IsSpace(config_body, char_ndx)) { ++char_ndx; }
  }

  void ConsumeWhitespace(string& config_body, size_t& char_ndx) {
    while (IsWhitespace(config_body, char_ndx)) { ++char_ndx; }
  }

  void ConsumeKeyChars(string& config_body, size_t& char_ndx) {
    while (IsKeyCharacter(config_body, char_ndx)) { ++char_ndx; }
  }

  // Functions that move a shared position forward and validate an expected token
  Status ConsumeEntrySeparator(string& config_body, size_t& char_ndx) {
    // >> Precondition: entry separator may have leading spaces only
    ConsumeSpaces(config_body, char_ndx);

    // >> Validation: entry separator must be '='
    if (not IsEntrySeparator(config_body, char_ndx)) {
      stringstream error_sstream;
      error_sstream << "Expected entry separator ('='); Found: "
                    << ParseLine(config_body, char_ndx)
      ;
      return Status::Invalid(error_sstream.str());
    }

    ConsumeSpaces(config_body, ++char_ndx);
    return Status::OK();
  }

  // Functions that look ahead for the next token/word
  string PeekLine(string& config_body, size_t& char_ndx) {
    size_t line_startndx = char_ndx;
    size_t peek_ndx      = char_ndx;
    while (not IsLineEnd(config_body, peek_ndx)) { ++peek_ndx; }

    return config_body.substr(line_startndx, peek_ndx - line_startndx);
  }

  // Functions that look ahead for the next token/word
  string PeekEntryKey(string& config_body, size_t& char_ndx) {
    size_t key_startndx = char_ndx;
    size_t peek_ndx     = char_ndx;
    ConsumeKeyChars(config_body, peek_ndx);

    return config_body.substr(key_startndx, peek_ndx - key_startndx);
  }

  // Functions that parse the string from the given position for a token/word
  string ParseLine(string& config_body, size_t& char_ndx) {
    size_t line_startndx = char_ndx;
    while (not IsLineEnd(config_body, char_ndx)) { ++char_ndx; }

    return config_body.substr(line_startndx, char_ndx++ - line_startndx);
  }

  string ParseEntryKey(string& config_body, size_t& char_ndx) {
    size_t key_startndx = char_ndx;
    ConsumeKeyChars(config_body, char_ndx);

    return config_body.substr(key_startndx, char_ndx++ - key_startndx);
  }

  Result<string> ParseEntryValScalarString(string& config_body, size_t& char_ndx) {
    // >> Precondition: String value must begin with "'"
    if (IsStringMarker(config_body, char_ndx)) { ++char_ndx; }
    else { return Status::Invalid("Value is not a string literal"); }

    size_t val_startndx = char_ndx;
    while (not IsStringMarker(config_body, char_ndx)) { ++char_ndx; }

    // >> Postcondition: String value must end with "'"
    if (char_ndx >= config_body.length()) {
      return Status::Invalid("Found unclosed string literal");
    }

    // and may have trailing whitespace only
    size_t val_stopndx = char_ndx++ - val_startndx;
    ConsumeWhitespace(config_body, char_ndx);

    return config_body.substr(val_startndx, val_stopndx);
  }

  Result<vector<string>>
  ParseEntryValListString(string& config_body, size_t& char_ndx) {
    // >> Precondition: String list must begin with '['
    if (IsListMarker(config_body, char_ndx)) { ++char_ndx; }
    else { return Status::Invalid("Value is not a list"); }

    vector<string> string_list;
    while (not IsListClose(config_body, char_ndx)) {
      ConsumeSpaces(config_body, char_ndx);

      ARROW_ASSIGN_OR_RAISE(
         string list_val
        ,ParseEntryValScalarString(config_body, char_ndx)
      );
      string_list.push_back(list_val);

      ConsumeSpaces(config_body, char_ndx);
      if (IsListSeparator(config_body, char_ndx)) { ++char_ndx; }
      else if (not IsListClose(config_body, char_ndx)) {
        return Status::Invalid("Expected list separator or close");
      }
    }

    // Move past the list close (']') and consume intermediate whitespace
    ConsumeWhitespace(config_body, ++char_ndx);

    return string_list;
  }

  Result<std::tuple<string, string>>
  ParseEntryString(string& config_body, size_t& char_ndx) {
    ConsumeSpaces(config_body, char_ndx);

    // >> Parsing
    string entry_key = ParseEntryKey(config_body, char_ndx);
    ConsumeSpaces(config_body, char_ndx);

    // Fail if separator is malformed
    ARROW_RETURN_NOT_OK(ConsumeEntrySeparator(config_body, char_ndx));

    ARROW_ASSIGN_OR_RAISE(
       string list_val
      ,ParseEntryValScalarString(config_body, char_ndx)
    );

    return std::make_tuple(entry_key, list_val);
  }

  Result<string> ParseSectionName(string& config_body, size_t& char_ndx) {
    // >> Precondition: A section name must start with '['
    if (not IsSectionHeader(config_body, char_ndx)) {
      return Status::Invalid("Not a section header");
    }

    // >> Parsing: Find the bounds of the section name (characters between '[' and ']')
    size_t name_startndx = ++char_ndx;
    while (not IsHeaderClose(config_body, char_ndx)) { ++char_ndx; }
    size_t name_stopndx  = char_ndx++ - name_startndx;

    // >> Postcondition: A section name may have trailing whitespace only
    ConsumeSpaces(config_body, char_ndx);
    if (IsLineEnd(config_body, char_ndx)) { ++char_ndx; }
    else { return Status::Invalid("Section header has invalid trailing characters"); }

    return config_body.substr(name_startndx, name_stopndx);
  }


  // >> Internal parsing functions
  Status ParseSectionSystem(string& config_body, size_t& char_ndx) {
    ARROW_ASSIGN_OR_RAISE(string section_name, ParseSectionName(config_body, char_ndx));
    if (section_name != "system") {
      return Status::Invalid("Expected section: system");
    }

    // NOTE: we only use these to check if parsing has so far failed
    ARROW_ASSIGN_OR_RAISE(auto data_tuple, ParseEntryString(config_body, char_ndx));
    ARROW_ASSIGN_OR_RAISE(auto meta_tuple, ParseEntryString(config_body, char_ndx));

    return Status::OK();
  }

  Status ParseSectionTopology( StorageHierarchy& topology
                              ,string&           config_body
                              ,size_t&           char_ndx) {
    ARROW_ASSIGN_OR_RAISE(string section_name, ParseSectionName(config_body, char_ndx));
    if (section_name != "topology") {
      return Status::Invalid("Expected section: topology");
    }

    // >> Parse entries until we hit the next section
    while (not IsHeaderOpen(config_body, char_ndx)) {
      // Parse the entry into a tuple
      ARROW_ASSIGN_OR_RAISE(auto entry_tuple, ParseEntryString(config_body, char_ndx));

      // Decompose the tuple into an engine name and it's location string
      auto [engine_label, engine_loc] = entry_tuple;
      ARROW_ASSIGN_OR_RAISE(auto parsed_loc, Location::Parse(engine_loc));

      // Update StorageHierarchy with information for this topology entry
      size_t engine_ndx = topology.labels.size();

      topology.labels.push_back(engine_label);
      topology.locations.push_back(parsed_loc);
      topology.configs.push_back(std::make_unique<ServiceConfig>());

      topology.engine_map[engine_label] = engine_ndx;

      // Update default-constructed ServiceConfig for this entry
      ServiceConfig& engine_cfg = *(topology.configs.back());
      engine_cfg.set_label(engine_label);
      engine_cfg.set_location(engine_loc);
    }

    return Status::OK();
  }

  Status ParseServiceConfigDownstream( string&           config_body
                                      ,size_t&           char_ndx
                                      ,StorageHierarchy& topology
                                      ,size_t            engine_ndx) {
    // Parse the key, which should match a ServiceConfig field
    string entry_key = ParseEntryKey(config_body, char_ndx);

    // Fail if separator is malformed
    ARROW_RETURN_NOT_OK(ConsumeEntrySeparator(config_body, char_ndx));

    // Parse the value
    ARROW_ASSIGN_OR_RAISE(
       vector<string> downstream_labels
      ,ParseEntryValListString(config_body, char_ndx)
    );

    ServiceConfig&  engine_cfg    = *(topology.configs[engine_ndx]);
    vector<size_t>& engine_dlinks = topology.downstream_links[engine_ndx];
    engine_dlinks.reserve(downstream_labels.size());

    // Make downstream and upstream links
    for (const string& dlabel : downstream_labels) {
      size_t downstream_ndx = topology.engine_map[dlabel];
      engine_dlinks.push_back(downstream_ndx);
      topology.upstream_links[downstream_ndx].push_back(engine_ndx);

      ServiceConfig& downstream_cfg = *(topology.configs[downstream_ndx]);
      engine_cfg.add_downstream()->CopyFrom(downstream_cfg);
    }

    return Status::OK();
  }

  Status ParseSectionService( string&           config_body
                             ,size_t&           char_ndx
                             ,StorageHierarchy& topology
                             ,size_t            engine_ndx) {
    // Parse the section name and validate it matches the expected engine label
    const string& engine_label = topology.labels[engine_ndx];
    ARROW_ASSIGN_OR_RAISE(string section_name, ParseSectionName(config_body, char_ndx));

    if (section_name != engine_label) {
      stringstream error_sstream;
      error_sstream << "Expected section [" << engine_label << "]";
      return Status::Invalid(error_sstream.str());
    }

    while (not IsHeaderOpen(config_body, char_ndx)) {
      // Prepare before parsing section body
      ConsumeWhitespace(config_body, char_ndx);

      if (IsEntryDownstream(config_body, char_ndx)) {
        ARROW_RETURN_NOT_OK(ParseServiceConfigDownstream(
          config_body, char_ndx, topology, engine_ndx
        ));
      }

      // This loops will keep going until we've exhausted the config
      else if (char_ndx == config_body.length()) { break; }
      else {
        stringstream error_sstream;
        error_sstream << "Invalid service entry:" << PeekLine(config_body, char_ndx);
        return Status::Invalid(error_sstream.str());
      }
    }

    return Status::OK();
  }

  // >> Static builder function for StorageHierarchy
  //! Builder for StorageHierarchy that reads from file `config_fpath`
  //  NOTE: this is mostly hardcoded for simplicity
  Result<unique_ptr<StorageHierarchy>>
  StorageHierarchy::FromFile(const char* config_fpath) {
    // The topology we will be building
    auto topology = std::make_unique<StorageHierarchy>();

    // Read the file into a string and set our position to the beginning
    string config_body;
    size_t char_ndx { 0 };

    if (not FileToString(config_fpath, config_body)) {
      std::cerr << "Failed to read data from topology config" << std::endl;
      return nullptr;
    }

    // Parse each section. This is hardcoded but somewhat flexible/extensible
    ARROW_RETURN_NOT_OK(ParseSectionSystem(config_body, char_ndx));
    ARROW_RETURN_NOT_OK(ParseSectionTopology(*topology, config_body, char_ndx));

    size_t count_services = topology->labels.size();
    topology->downstream_links = vector<vector<size_t>>(count_services);
    topology->upstream_links   = vector<vector<size_t>>(count_services);
    
    // Parse a section for each entry in [topology]
    // Precondition: Each service section is in same order as [topology] entries
    for (size_t engine_ndx = 0; engine_ndx < topology->labels.size(); ++engine_ndx) {
      ARROW_RETURN_NOT_OK(ParseSectionService(config_body, char_ndx, *topology, engine_ndx));
    }

    return topology;
  }

  void StringifyServiceInfo( stringstream&           sstream
                            ,string                  prefix
                            ,const StorageHierarchy& topology
                            ,size_t                  engine_ndx) {
    const vector<size_t>& dlinks = topology.downstream_links[engine_ndx];
    if (dlinks.empty()) { return; }

    for (size_t dlink_ndx = 0; dlink_ndx < dlinks.size(); ++dlink_ndx) {
      size_t        dlink_engine = dlinks[dlink_ndx];
      const string& engine_label = topology.labels[dlink_engine];

      if (dlink_ndx == dlinks.size() - 1) { sstream << prefix <<    leaf_prefix; }
      else                                { sstream << prefix << nonleaf_prefix; }

      sstream << link_symbol << engine_label << std::endl;
      StringifyServiceInfo(sstream, base_prefix + prefix, topology, dlink_engine);
    }
  }

  void StorageHierarchy::PrintTopology() {
    stringstream   print_stream;
    vector<size_t> root_engines;

    // Print all top-level locations
    print_stream << "Top-level engines:" << std::endl;
    for (size_t engine_ndx = 0; engine_ndx < upstream_links.size(); ++engine_ndx) {
      if (not upstream_links[engine_ndx].empty()) { continue; }

      root_engines.push_back(engine_ndx);
      print_stream << "\t" << "[" << labels[engine_ndx] << "]"
                   << " "  << locations[engine_ndx].ToString()
                   << std::endl
      ;

      break;
    }

    // Then print the hierarchy
    print_stream << std::endl << "Storage hierarchy:" << std::endl;
    for (size_t engine_ndx : root_engines) {
      print_stream << "." << labels[engine_ndx] << std::endl;
      StringifyServiceInfo(print_stream, empty_prefix, *this, engine_ndx);
    }

    std::cout << print_stream.str() << std::endl;
  }

  Result<size_t> StorageHierarchy::IndexForLabel(string& engine_label) {
    size_t engine_ndx = 0;
    for (; engine_ndx < labels.size(); ++engine_ndx) {
      if (engine_label == labels[engine_ndx]) { return engine_ndx; }
    }

    return Status::Invalid("Requested unregistered engine name");
  }

  Result<size_t> StorageHierarchy::IndexForLocation(Location& engine_loc) {
    size_t engine_ndx = 0;
    for (; engine_ndx < locations.size(); ++engine_ndx) {
      if (engine_loc == locations[engine_ndx]) { return engine_ndx; }
    }

    return Status::Invalid("Requested unregistered location");
  }

  Status
  StorageHierarchy::UpdateConfigView(size_t upstream_ndx, size_t engine_ndx) {
    size_t dlink_pos = 0;

    // For each downstream link from the upstream engine
    for (size_t dlink_ndx : downstream_links[upstream_ndx]) {
      // Find the link corresponding to engine_ndx
      // Then, update the upstream's copy of the engine's config
      if (dlink_ndx == engine_ndx) {
        ServiceConfig* view_cfg = configs[upstream_ndx]->mutable_downstream(dlink_pos);
        view_cfg->CopyFrom(*(configs[engine_ndx]));

        return Status::OK();
      }

      ++dlink_pos;
    }

    return Status::Invalid("Could not find downstream link to update");
  }

} // namespace: skytether::services


// ------------------------------
// Classes

// >> Hash functor implementations
namespace skytether::services {

  std::size_t HashFunctorSkytetherTicket::operator()(const Ticket& skyticket) const {
    // A skytether ticket may be a service location or a query identifier
    static std::hash<string> HashSkytetherTicketId;

    return HashSkytetherTicketId(skyticket.ticket);
  }

  std::size_t HashFunctorSkytetherLocation::operator()(const Location& skyloc) const {
    // A location is essentially a URI
    static std::hash<string> HashSkytetherLocation;

    return HashSkytetherLocation(skyloc.ToString());
  }

} // namespace: skytether::services


// >> TopologyService implementations
namespace skytether::services {

  // |> Helper functions
  Result<FlightEndpoint>
  TopologyService::GetDownstreamServices([[maybe_unused]] FlightEndpoint& upstream_srv) {
    return Status::NotImplemented("WIP");
  }

  // >> Custom Flight API
  // TODO: this is bound to be all fucked up
  Status
  TopologyService::DoActivateService( [[maybe_unused]] const ServerCallContext&  context
                                     ,                 const shared_ptr<Buffer>  serialized_loc
                                     ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
    SkytetherDebugMsg("Handling request: [register-service]");

    // Deserialize the location URI and parse it into a `Location`
    string location_uri = serialized_loc->ToString();
    ARROW_ASSIGN_OR_RAISE(auto service_loc, Location::Parse(location_uri));
    
    // Verify the location exists and is inactive (not yet claimed by another device)
    ARROW_ASSIGN_OR_RAISE(size_t engine_ndx, service_map->IndexForLocation(service_loc));

    ServiceConfig& engine_cfg = *(service_map->configs[engine_ndx]);
    if (engine_cfg.is_active()) { return Status::Invalid("Location already active"); }

    // Activate the configuration and send it in the response
    SkytetherDebugMsg("Registering location [" << service_loc.ToString() << "]");
    engine_cfg.set_is_active(true);

    string response_payload;
    if (not engine_cfg.SerializeToString(&response_payload)) {
      return Status::Invalid("Unable to serialize config for response");
    }

    // Create a response stream with a single buffer containing the payload
    (*response_stream) = std::make_unique<SimpleResultStream>(
      vector<FlightResult> { FlightResult { Buffer::FromString(response_payload) } }
    );

    // If there is an upstream service, send it a view change
    for (size_t upstream_ndx : service_map->upstream_links[engine_ndx]) {
      Location&      upstream_loc = service_map->locations[upstream_ndx];
      ServiceConfig& upstream_cfg = *(service_map->configs[upstream_ndx]);

      // Update the upstream's copy of this engine's config
      ARROW_RETURN_NOT_OK(service_map->UpdateConfigView(upstream_ndx, engine_ndx));

      // Then send it
      SkytetherDebugMsg("Connecting to service [" << upstream_loc.ToString() << "]");
      auto skyconn = SkytetherClient::ForLocation(upstream_loc);
      if (skyconn == nullptr) { return Status::Invalid("Unable to connect to service"); }

      SkytetherDebugMsg("Sending view change");
      ARROW_RETURN_NOT_OK(skyconn->SendViewUpdate(upstream_cfg));
    }

    return Status::OK();
  }

  Status
  TopologyService::DoDeactivateService( [[maybe_unused]] const ServerCallContext&  context
                                       ,                 const shared_ptr<Buffer>  serialized_loc
                                       ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
    SkytetherDebugMsg("Handling request: [" << ActionDeactivate << "]");

    // Deserialize location URI and parse it into a `Location`
    string location_uri = serialized_loc->ToString();
    ARROW_ASSIGN_OR_RAISE(auto service_loc, Location::Parse(location_uri));
    
    // Verify we are de-registering a previously registered location
    ARROW_ASSIGN_OR_RAISE(size_t engine_ndx, service_map->IndexForLocation(service_loc));
    ServiceConfig& engine_cfg = *(service_map->configs[engine_ndx]);
    if (not engine_cfg.is_active()) {
      return Status::Invalid("Location already inactive");
    }

    SkytetherDebugMsg("De-activating location [" << service_loc.ToString() << "]");
    engine_cfg.set_is_active(false);

    // If there is an upstream service, send it a view change
    for (size_t upstream_ndx : service_map->upstream_links[engine_ndx]) {
      Location&      upstream_loc = service_map->locations[upstream_ndx];
      ServiceConfig& upstream_cfg = *(service_map->configs[upstream_ndx]);

      // Update the upstream's copy of this engine's config
      ARROW_RETURN_NOT_OK(service_map->UpdateConfigView(upstream_ndx, engine_ndx));

      // Then send it
      auto client_conn = SkytetherClient::ForLocation(upstream_loc);
      if (client_conn == nullptr) {
        return Status::Invalid("Unable to connect to service");
      }

      ARROW_RETURN_NOT_OK(client_conn->SendViewUpdate(upstream_cfg));
      SkytetherDebugMsg("Sent view change to [" << upstream_loc.ToString() << "]");
    }

    return Status::OK();
  }

  Status
  TopologyService::DoDisableDecomposition( [[maybe_unused]] const ServerCallContext&  context
                                          ,                 const shared_ptr<Buffer>  serialized_loc
                                          ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
    SkytetherDebugMsg("Handling request: [" << ActionDisableDecomp << "]");

    // Deserialize location URI and parse it into a `Location`
    string location_uri = serialized_loc->ToString();
    ARROW_ASSIGN_OR_RAISE(auto engine_loc, Location::Parse(location_uri));
    
    // Verify we are updating a previously registered location
    ARROW_ASSIGN_OR_RAISE(size_t engine_ndx, service_map->IndexForLocation(engine_loc));

    ServiceConfig& engine_cfg = *(service_map->configs[engine_ndx]);
    if (not engine_cfg.is_active()) { return Status::Invalid("Location inactive"); }

    // Localize the updated configuration
    engine_cfg.set_decompose_alg(DecomposeAlg::None);

    // Send the update to the service
    auto client_conn = SkytetherClient::ForLocation(engine_loc);
    if (client_conn == nullptr) { return Status::Invalid("Unable to connect to service"); }

    ARROW_RETURN_NOT_OK(client_conn->SendViewUpdate(engine_cfg));
    SkytetherDebugMsg("Sent updated configuration to [" << engine_loc.ToString() << "]");

    return Status::OK();
  }

  Status
  TopologyService::DoEnableDecomposition( [[maybe_unused]] const ServerCallContext&  context
                                         ,                 const shared_ptr<Buffer>  serialized_loc
                                         ,[[maybe_unused]] unique_ptr<ResultStream>* response_stream) {
    SkytetherDebugMsg("Handling request: [" << ActionEnableDecomp << "]");

    // Deserialize location URI and parse it into a `Location`
    string location_uri = serialized_loc->ToString();
    ARROW_ASSIGN_OR_RAISE(auto engine_loc, Location::Parse(location_uri));
    
    // Verify we are updating a previously registered location
    ARROW_ASSIGN_OR_RAISE(size_t engine_ndx, service_map->IndexForLocation(engine_loc));
    ServiceConfig& engine_cfg = *(service_map->configs[engine_ndx]);
    if (not engine_cfg.is_active()) { return Status::Invalid("Location inactive"); }

    // Localize the updated configuration
    // engine_cfg.set_decompose_alg(DecomposeAlg::WideJoinHead);
    engine_cfg.set_decompose_alg(DecomposeAlg::Eager);

    // Send the update to the service
    auto client_conn = SkytetherClient::ForLocation(engine_loc);
    if (client_conn == nullptr) { return Status::Invalid("Unable to connect to service"); }

    ARROW_RETURN_NOT_OK(client_conn->SendViewUpdate(engine_cfg));
    SkytetherDebugMsg("Sent updated configuration to [" << engine_loc.ToString() << "]");

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

    else if (action.type == ActionDisableDecomp) {
      return DoDisableDecomposition(context, action.body, result);
    }

    else if (action.type == ActionEnableDecomp) {
      return DoEnableDecomposition(context, action.body, result);
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

} // namespace: skytether::services
