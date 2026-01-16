#include "RenderGraphCompiler.h"

#include "RenderGraph.h"
#include "Common/RHISemaphore.h"

void RenderGraphTopology::Initialize(size_t node_count)
{
    heads.assign(node_count, InvalidID);
    degrees.assign(node_count, DefaultID);
}

void RenderGraphTopology::AddEdge(int32_t from, int32_t to)
{
    int32_t new_edge_index = static_cast<int32_t>(edges.size());
    edges.push_back({ to, heads[from] });
    heads[from] = new_edge_index;
    degrees[to]++;
}

void RenderGraphDependency::Initialize(size_t texture_count, size_t buffer_count)
{
    texture_last_writer.resize(texture_count, InvalidID);
    texture_last_reader.resize(texture_count);
    buffer_last_writer.resize(buffer_count, InvalidID);
    buffer_last_reader.resize(buffer_count);
}

RenderGraphCompiler::RenderGraphCompiler(RenderGraph& in_render_graph)
    : render_graph(in_render_graph)
{
    topology.Initialize(render_graph.pass_data.size());
    dependency.Initialize(render_graph.textures.size(), render_graph.buffers.size());
}

void RenderGraphCompiler::Compile()
{
    Analyze();
    Sort();
    Synchronize();
}

const RenderGraphTimeline & RenderGraphCompiler::GetTimeline()
{
    return timeline;
}

void RenderGraphCompiler::Analyze()
{
    for (uint32_t pass_index = 0; pass_index < render_graph.pass_data.size(); pass_index++)
    {
        const auto& pass = render_graph.pass_data[pass_index];

        for (const auto& texture_handle : pass.texture_reads)
        {
            ProcessRead(pass_index, texture_handle.index, dependency.texture_last_writer, dependency.texture_last_reader);
        }

        for (const auto& buffer_handle : pass.buffer_reads)
        {
            ProcessRead(pass_index, buffer_handle.index, dependency.buffer_last_writer, dependency.buffer_last_reader);
        }

        for (const auto& texture_handle : pass.texture_writes)
        {
            ProcessWrite(pass_index, texture_handle.index, dependency.texture_last_writer, dependency.texture_last_reader);
        }

        for (const auto& buffer_handle : pass.buffer_writes)
        {
            ProcessWrite(pass_index, buffer_handle.index, dependency.buffer_last_writer, dependency.buffer_last_reader);
        }
    }
}

void RenderGraphCompiler::Sort()
{
    timeline.topology_order.clear();
    timeline.topology_order.reserve(render_graph.pass_data.size());

    std::vector<int32_t> queue;
    queue.reserve(render_graph.pass_data.size());

    for (size_t index = 0; index < topology.degrees.size(); ++index)
    {
        if (topology.degrees[index] == DefaultID)
        {
            queue.push_back(index);
        }
    }

    size_t head = 0;
    while (head < queue.size())
    {
        int32_t u = queue[head++];
        timeline.topology_order.push_back(u);

        int32_t edge_index = topology.heads[u];
        while (edge_index != InvalidID)
        {
            const auto& edge = topology.edges[edge_index];
            int32_t v = edge.target_node;

            topology.degrees[v]--;
            if (topology.degrees[v] == DefaultID)
            {
                queue.push_back(v);
            }

            edge_index = edge.next_edge_index;
        }
    }

    ensure(timeline.topology_order.size() == render_graph.pass_data.size());

    timeline.texture_lifetimes.assign(render_graph.textures.size(), {});
    timeline.buffer_lifetimes.assign(render_graph.buffers.size(), {});

    for (int32_t time_step = 0; time_step < static_cast<int32_t>(timeline.topology_order.size()); ++time_step)
    {
        int32_t physical_pass_index = timeline.topology_order[time_step];
        const auto& pass = render_graph.pass_data[physical_pass_index];

        for (const auto& texture_handle : pass.texture_reads)
        {
            if (timeline.texture_lifetimes[texture_handle.index].first_pass_index == InvalidID)
            {
                timeline.texture_lifetimes[texture_handle.index].first_pass_index = time_step;
            }

            timeline.texture_lifetimes[texture_handle.index].last_pass_index = time_step;
        }

        for (const auto& texture_handle : pass.texture_writes)
        {
            if (timeline.texture_lifetimes[texture_handle.index].first_pass_index == InvalidID)
            {
                timeline.texture_lifetimes[texture_handle.index].first_pass_index = time_step;
            }

            timeline.texture_lifetimes[texture_handle.index].last_pass_index = time_step;
        }

        for (const auto& buffer_handle : pass.buffer_reads)
        {
            if (timeline.buffer_lifetimes[buffer_handle.index].first_pass_index == InvalidID)
            {
                timeline.buffer_lifetimes[buffer_handle.index].first_pass_index = time_step;
            }

            timeline.buffer_lifetimes[buffer_handle.index].last_pass_index = time_step;
        }

        for (const auto& buffer_handle : pass.buffer_writes)
        {
            if (timeline.buffer_lifetimes[buffer_handle.index].first_pass_index == InvalidID)
            {
                timeline.buffer_lifetimes[buffer_handle.index].first_pass_index = time_step;
            }

            timeline.buffer_lifetimes[buffer_handle.index].last_pass_index = time_step;
        }
    }
}

void RenderGraphCompiler::Synchronize()
{
    for (size_t index = 0; index < render_graph.pass_data.size(); ++index)
    {
        if (!render_graph.textures.empty())
		{
			const auto& texture_resource = render_graph.textures[index];
			const auto& texture_lifetime = timeline.texture_lifetimes[index];

			if (!texture_resource.external_wait_semaphore.empty() && texture_lifetime.first_pass_index != InvalidID)
			{
				auto& pass = render_graph.pass_data[texture_lifetime.first_pass_index];
				pass.wait_semaphores.insert(pass.wait_semaphores.end(), texture_resource.external_wait_semaphore.begin(), texture_resource.external_wait_semaphore.end());
			}

			if (!texture_resource.external_signal_semaphore.empty() && texture_lifetime.last_pass_index != InvalidID)
			{
				auto& pass = render_graph.pass_data[texture_lifetime.last_pass_index];
				pass.signal_semaphores.insert(pass.signal_semaphores.end(), texture_resource.external_signal_semaphore.begin(), texture_resource.external_signal_semaphore.end());
			}
		}

        if (!render_graph.buffers.empty())
		{
			const auto& buffer_resource = render_graph.buffers[index];
			const auto& buffer_lifetime = timeline.texture_lifetimes[index];

			if (!buffer_resource.external_wait_semaphore.empty() && buffer_lifetime.first_pass_index != InvalidID)
			{
				auto& pass = render_graph.pass_data[buffer_lifetime.first_pass_index];
				pass.wait_semaphores.insert(pass.wait_semaphores.end(), buffer_resource.external_wait_semaphore.begin(), buffer_resource.external_wait_semaphore.end());
			}

			if (!buffer_resource.external_signal_semaphore.empty() && buffer_lifetime.last_pass_index != InvalidID)
			{
				auto& pass = render_graph.pass_data[buffer_lifetime.last_pass_index];
				pass.signal_semaphores.insert(pass.signal_semaphores.end(), buffer_resource.external_signal_semaphore.begin(), buffer_resource.external_signal_semaphore.end());
			}
		}
    }
}

void RenderGraphCompiler::ProcessRead(uint32_t pass_index, uint32_t resource_index,
                                      std::vector<int32_t> &last_writers,
                                      std::vector<std::vector<int32_t>> &last_readers)
{
    if (resource_index >= last_writers.size())
    {
        return;
    }

    int32_t writer = last_writers[resource_index];
    if (writer != InvalidID)
    {
        topology.AddEdge(writer, pass_index);
    }

    last_readers[resource_index].push_back(static_cast<int>(pass_index));
}

void RenderGraphCompiler::ProcessWrite(uint32_t pass_index, uint32_t resource_index,
    std::vector<int32_t> &last_writers,
    std::vector<std::vector<int32_t>> &last_readers)
{
    if (resource_index >= last_writers.size())
    {
        return;
    }

    int32_t writer = last_writers[resource_index];
    if (writer != InvalidID)
    {
        topology.AddEdge(writer, pass_index);
    }

    for (int32_t reader : last_readers[resource_index])
    {
        if (reader != static_cast<int32_t>(pass_index))
        {
            topology.AddEdge(reader, pass_index);
        }
    }

    last_writers[resource_index] = static_cast<int32_t>(pass_index);
    last_readers[resource_index].clear();
}
