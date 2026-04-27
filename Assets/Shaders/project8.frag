struct GSOutput {
    float4 pos : SV_Position;
};

float4 main(GSOutput input) : SV_Target {
    return float4(0.0, 1.0, 0.0, 1.0);
}