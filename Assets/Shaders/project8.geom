struct DSOutput {
    float4 pos : SV_Position;
    [[vk::location(0)]] float4 PosL : TEXCOORD0;
    [[vk::location(1)]] float2 TexUV : TEXCOORD2;
};

struct GSOutput {
    float4 pos : SV_Position;
    float2 TexUV : TEXCOORD2;
};

[maxvertexcount(4)]
void main(triangle DSOutput input[3], inout LineStream<GSOutput> outStream) {
    GSOutput output;

    output.pos = input[0].pos;
    output.TexUV = input[0].TexUV;
    outStream.Append(output);

    output.pos = input[1].pos;
    output.TexUV = input[1].TexUV;
    outStream.Append(output);

    output.pos = input[2].pos;
    output.TexUV = input[2].TexUV;
    outStream.Append(output);

    output.pos = input[0].pos;
    output.TexUV = input[0].TexUV;
    outStream.Append(output);

    outStream.RestartStrip();
}