#include "RenderObject.h"

#include "MaterialDataBuffer.h"
#include "BindlessDescriptorSet.h"

void MeshNode::Draw(const glm::mat4 &topMatrix, std::vector<RenderObject> &ctx) {
    glm::mat4 nodeMatrix = topMatrix * worldTransform_;
    for (auto &s: meshAsset_->GetSurfaces()) {
        ctx.emplace_back(
            s.GetCount(),
            s.GetStartIndex(),
            meshAsset_->GetMeshBuffer().GetIndexBuffer()->GetBuffer(),
            nodeMatrix,
            meshAsset_->GetMeshBuffer().GetBufferAddress(),
            s.GetMaterial()
        );
    }
    RenderNode::Draw(topMatrix, ctx);
}

void RenderObject::Draw(VkCommandBuffer cmd, VkDeviceAddress invDataAddr, VkDeviceAddress sceneDataAddr) {
    vkCmdBindIndexBuffer(cmd, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

    DrawPushConstants pushConstants{
        .modelMatrix = modelMatrix_,
        .invData = invDataAddr,
        .vertexBuffer = vertexBufferAddr_,
        .materialDataBuffer = MaterialDataBuffer::GetInstance().GetBuffer()->GetDeviceAddress() +
                              materialInstance_->GetDataBufferOffset(),
        .sceneDataBuffer = sceneDataAddr
    };
    vkCmdPushConstants(cmd, materialInstance_->GetMaterialPipeline()->GetVkPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0,
                       sizeof(DrawPushConstants), &pushConstants);

    vkCmdDrawIndexed(cmd, indexCount_, 1, firstIndex_, 0, 0);
}
