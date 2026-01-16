#pragma once

#include <cstdint>
#include <vector>

static constexpr int32_t DefaultID = 0;
static constexpr int32_t InvalidID = -1;

struct RenderGraphTopology
{
    struct Edge
    {
        // Node this edge points to
        int32_t target_node;

        // Index of the next edge in the array
        int32_t next_edge_index;
    };

    std::vector<Edge> edges;
    std::vector<int32_t> heads;
    std::vector<int32_t> degrees;

    void Initialize(size_t node_count);
    void AddEdge(int32_t from, int32_t to);
};

struct RenderGraphDependency
{
    std::vector<std::vector<int32_t>> texture_last_reader;
    std::vector<std::vector<int32_t>> buffer_last_reader;
    std::vector<int32_t> texture_last_writer;
    std::vector<int32_t> buffer_last_writer;

    void Initialize(size_t texture_count, size_t buffer_count);
};

struct RenderGraphTimeline
{
    struct ResourceLifetime
    {
        int32_t first_pass_index = InvalidID;
        int32_t last_pass_index = InvalidID;
    };

    std::vector<int32_t> topology_order;
    std::vector<ResourceLifetime> texture_lifetimes;
    std::vector<ResourceLifetime> buffer_lifetimes;

    void Initialize();
};

class RenderGraph;

class RenderGraphCompiler
{
    RenderGraph& render_graph;

public:
    RenderGraphCompiler(RenderGraph& in_render_graph);

    void Compile();

    const RenderGraphTimeline& GetTimeline();

private:
    void Analyze();
    void Sort();
    void Synchronize();

    void ProcessRead(uint32_t pass_index, uint32_t resource_index, std::vector<int32_t>& last_writers, std::vector<std::vector<int32_t>>& last_readers);
    void ProcessWrite(uint32_t pass_index, uint32_t resource_index, std::vector<int32_t>& last_writers, std::vector<std::vector<int32_t>>& last_readers);

private:
    RenderGraphTopology topology;
    RenderGraphDependency dependency;
    RenderGraphTimeline timeline;
};
