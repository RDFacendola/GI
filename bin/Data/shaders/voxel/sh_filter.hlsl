/// \brief Used to fill the missing infos of a MIP level by using the next one.
/// During light injection only the most detailed MIP levels are filled with useful informations,
/// this leaves each MIP with cube (gVoxelResolution\2)^3 empty on the center of that MIP.
/// Downscales the next MIP level to fill that area up to the base of the pyramid.

#define N 8
#define TOTAL_THREADS (N * N * N)


#include "voxel_def.hlsl"

cbuffer SHFilter {

    int3 gSrcOffset;					// Offset of the surface used as source of the filtering.
    int gSrcStride;						// Horizontal stride for each source SH band coefficient.

    int3 gDstOffset;					// Offset of the surface used as destination of the filtering.	
    int gDstStride;						// Horizontal stride for each destination SH band coefficient.

    int3 gMIPOffset;					// Offset to apply within a single filtered MIP.
    int padding;						
        
};

groupshared int samples[N][N][N];		// Store the samples of the source texture.

void FilterSum(RWTexture3D<int> surface, int3 thread, int3 group_thread) {

    // Sample everything and store in group shared memory

    samples[group_thread.x][group_thread.y][group_thread.z] = surface[thread + gSrcOffset];

    GroupMemoryBarrierWithGroupSync();

    // Filter (downscale)

    if (group_thread.x % 2 == 0 &&
        group_thread.y % 2 == 0 &&
        group_thread.z % 2 == 0) {

        int sh_coefficient_index = thread.x / gSrcStride;

        thread.x %= gSrcStride;
        
        int3 dst_offset = gDstOffset + gMIPOffset;
        
        dst_offset.x += gDstStride * sh_coefficient_index;

        surface[thread / 2 + dst_offset] =   samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 0]
                                           + samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 1]
                                           + samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 0]
                                           + samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 1]
                                           + samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 0]
                                           + samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 1]
                                           + samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 0]
                                           + samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 1];

    }

}

/// \brief Performs alpha multiplication, considering that the opacity is stored in 1024th.
int AlphaMul(int a, int b) {

    float fa = a / 1024.0f;
    
    return a + (1.0f - fa) * b;

}

void FilterOpacity(RWTexture3D<int> surface, int3 thread, int3 group_thread) {

    // Sample everything and store in group shared memory

    samples[group_thread.x][group_thread.y][group_thread.z] = surface[thread + gSrcOffset];

    GroupMemoryBarrierWithGroupSync();

    // Filter (downscale)

    if (group_thread.x % 2 == 0 &&
        group_thread.y % 2 == 0 &&
        group_thread.z % 2 == 0) {

        int sh_coefficient_index = thread.x / gSrcStride;

        thread.x %= gSrcStride;
        
        int3 dst_offset = gDstOffset + gMIPOffset;
        
        dst_offset.x += gDstStride * sh_coefficient_index;

        if (sh_coefficient_index == 1) {

            // Opacity along X
            surface[thread / 2 + dst_offset] = (   AlphaMul( samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 0],
                                                             samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 0] )
                                                 + AlphaMul( samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 1],
                                                             samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 1] )
                                                 + AlphaMul( samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 0],
                                                             samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 0] )
                                                 + AlphaMul( samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 1],
                                                             samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 1] ) ) >> 2;

        }
        else if (sh_coefficient_index == 2) {

            // Opacity along Y

            surface[thread / 2 + dst_offset] = (   AlphaMul( samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 0],
                                                             samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 0] )
                                                 + AlphaMul( samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 1],
                                                             samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 1] )
                                                 + AlphaMul( samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 0],
                                                             samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 0] )
                                                 + AlphaMul( samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 1],
                                                             samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 1] ) ) >> 2;

        }
        else if(sh_coefficient_index == 3){

            // Opacity along Z

            surface[thread / 2 + dst_offset] = (   AlphaMul( samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 0],
                                                             samples[group_thread.x + 0][group_thread.y + 0][group_thread.z + 1] )
                                                 + AlphaMul( samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 0],
                                                             samples[group_thread.x + 0][group_thread.y + 1][group_thread.z + 1] )
                                                 + AlphaMul( samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 0],
                                                             samples[group_thread.x + 1][group_thread.y + 0][group_thread.z + 1] )
                                                 + AlphaMul( samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 0],
                                                             samples[group_thread.x + 1][group_thread.y + 1][group_thread.z + 1] ) ) >> 2;

        }
        else {

            // Isotropic opacity. Not used.

            surface[thread / 2 + dst_offset] = 0;

        }

    }

}

[numthreads(N, N, N)]
void CSMain(uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID) {

    // Red
    FilterSum(gSHRed, thread_id, group_thread_id);

    GroupMemoryBarrierWithGroupSync();

    // Green
    FilterSum(gSHGreen, thread_id, group_thread_id);

    GroupMemoryBarrierWithGroupSync();

    // Blue
    FilterSum(gSHBlue, thread_id, group_thread_id);
    
    GroupMemoryBarrierWithGroupSync();

    // Opacity
    FilterOpacity(gSHAlpha, thread_id, group_thread_id);

    //GroupMemoryBarrierWithGroupSync();

}