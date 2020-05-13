#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/serialization/access.hpp>

#include <graphtyper/constants.hpp>
#include <graphtyper/graph/absolute_position.hpp>
#include <graphtyper/graph/genomic_region.hpp>
#include <graphtyper/graph/haplotype.hpp>
#include <graphtyper/graph/location.hpp>
#include <graphtyper/graph/node.hpp>
#include <graphtyper/graph/sv.hpp>
#include <graphtyper/index/kmer_label.hpp>
#include <graphtyper/typer/path.hpp>


namespace gyper
{

class Path;
class Variant;
class VarRecord;


struct Contig
{
  std::string name{};
  uint32_t length{0};

  template <typename Archive>
  void
  serialize(Archive & ar, unsigned int)
  {
    ar & name;
    ar & length;
  }


};

class Graph
{
  friend class boost::serialization::access; // boost is my friend, allow him to see my privates

public:
  bool use_absolute_positions{true};
  bool is_sv_graph{false};
  GenomicRegion genomic_region;
  std::vector<char> reference;
  //uint32_t reference_offset{0};
  AbsolutePosition absolute_pos;
  std::vector<RefNode> ref_nodes;
  std::vector<VarNode> var_nodes;
  std::vector<SV> SVs;
  std::vector<Contig> contigs;

  /****************
   * CONSTRUCTORS *
   ****************/
  explicit
  Graph(bool _use_absolute_positions = true);

  void clear();
  void add_genomic_region(std::vector<char> && reference_sequence,
                          std::vector<VarRecord> && var_records,
                          GenomicRegion && region);

  /******************
   * GRAPH CREATION *
   ******************/
  void generate_reference_genome();
  void create_special_positions();

  /****************
   * GRAPH ACCESS *
   ****************/
  std::vector<char> get_all_ref() const;
  std::vector<char> get_ref(uint32_t from, uint32_t to) const;
  std::vector<char> get_generated_reference_genome(uint32_t & from, uint32_t & to) const;
  std::vector<char> get_reference_ref(uint32_t & from, uint32_t & to) const;
  std::vector<char> get_first_var() const;
  std::vector<char> walk_random_path(uint32_t from, uint32_t to) const;

  /*********************
   * GRAPH INFORMATION *
   *********************/
  std::size_t size() const;
  uint32_t get_variant_order(long variant_id) const;
  uint16_t get_variant_num(uint32_t v) const;
  std::vector<Haplotype> get_all_haplotypes(uint32_t variant_distance = MAX_READ_LENGTH) const;

  std::vector<char> get_sequence_of_a_haplotype_call(std::vector<Genotype> const & gts,
                                                     uint32_t const haplotype_call) const;

  std::vector<std::vector<char> > get_all_sequences_of_a_genotype(Genotype const & gt) const;

  std::vector<std::vector<char> >
  get_sequence_from_location(Location const & loc,
                             uint32_t const length,
                             std::vector<char> const & prefix
                             ) const;

  bool is_variant_in_graph(Variant const & var) const;
  uint8_t get_10log10_num_paths(TNodeIndex const v, uint32_t const MAX_DISTANCE = 60);
  bool is_snp(Genotype const & gt) const;
  bool is_homopolymer(Genotype const & gt) const;

  /*************************
   * GRAPH LOCAL ALIGNMENT *
   *************************/
  std::unordered_set<long>
  reference_distance_between_locations(std::vector<Location> const & ll1,
                                       std::vector<Location> const & ll2) const;

  std::vector<Location> get_locations_of_a_position(uint32_t pos, Path const & path) const;
  std::vector<Location> get_locations_of_an_actual_position(uint32_t pos,
                                                            Path const & path,
                                                            bool const is_special = false) const;

  std::vector<KmerLabel>
  get_labels_forward(Location const & s,
                     std::vector<char> const & read,
                     uint32_t & max_mismatches) const;

  std::vector<KmerLabel>
  get_labels_backward(Location const & e,
                      std::vector<char> const & read,
                      uint32_t & max_mismatches) const;

  std::vector<KmerLabel>
  iterative_dfs(std::vector<Location> const & start_locations,
                std::vector<Location> const & end_locations,
                std::vector<char> const & read,
                uint32_t & max_mismatches) const;

  /*********************
   * SPECIAL POSITIONS *
   *********************/
  void add_special_pos(uint32_t reach, uint32_t ref_reach);
  bool is_special_pos(uint32_t pos) const;
  uint32_t get_special_pos(uint32_t pos, uint32_t ref_reach) const;
  uint32_t get_ref_reach_pos(uint32_t pos) const;
  uint32_t get_actual_pos(uint32_t pos) const;

  std::unordered_map<uint32_t, std::vector<uint32_t> > ref_reach_to_special_pos;
  std::vector<uint32_t> ref_reach_poses;
  std::vector<uint32_t> actual_poses;

  /**
   * ERROR CHECKING
   */
  bool check() const;
  bool check_ACGTN_only() const;
  bool check_empty_variant_dna() const;
  bool check_increasing_order() const;
  bool check_if_order_follows_reference() const;

  /**
   * Other
   */
  std::vector<uint32_t> get_var_orders(uint32_t const start, uint32_t const end) const;
  void print() const;

private:
  template <typename Archive>
  void serialize(Archive & ar, unsigned int);

  /**********************
   * GRAPH MODIFICATION *
   **********************/
  bool add_reference(unsigned end_pos,
                     unsigned const & num_var,
                     std::vector<char> const & reference_sequence);

  void add_variants(VarRecord && record);

  void break_apart_haplotypes(std::vector<Genotype> gts,
                              std::vector<Haplotype> & haplotypes,
                              int32_t max_read_length
                              ) const;
};

extern Graph graph;

} // namespace gyper
