#include "Landscape/SynthwaveCloud.h"
#include "Kismet/KismetMathLibrary.h"

ASynthwaveCloud::ASynthwaveCloud()
{
    PrimaryActorTick.bCanEverTick = true;

    cloud_mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CloudMesh"));
    RootComponent = cloud_mesh;
    cloud_mesh->bUseAsyncCooking = true;
    cloud_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASynthwaveCloud::BeginPlay()
{
    Super::BeginPlay();
    generate_low_poly_cloud();
}

void ASynthwaveCloud::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AddActorWorldOffset(movement_direction * move_speed * DeltaTime);
    AddActorLocalRotation(rotation_speed * DeltaTime);

    lifetime_timer += DeltaTime;
    if (lifetime_timer >= lifetime_seconds)
    {
        Destroy();
    }
}

void create_icosahedron(TArray<FVector>& vertices, TArray<FIntVector>& faces)
{
    const float t = (1.0 + FMath::Sqrt(5.0)) / 2.0;
    vertices = {
        FVector(-1,  t,  0), FVector(1,  t,  0), FVector(-1, -t,  0), FVector(1, -t,  0),
        FVector(0, -1,  t), FVector(0,  1,  t), FVector(0, -1, -t), FVector(0,  1, -t),
        FVector(t,  0, -1), FVector(t,  0,  1), FVector(-t,  0, -1), FVector(-t,  0,  1)
    };

    for (FVector& v : vertices)
        v = v.GetSafeNormal();  // unit radius

    faces = {
        {0,11,5}, {0,5,1}, {0,1,7}, {0,7,10}, {0,10,11},
        {1,5,9}, {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
        {3,9,4}, {3,4,2}, {3,2,6}, {3,6,8}, {3,8,9},
        {4,9,5}, {2,4,11}, {6,2,10}, {8,6,7}, {9,8,1}
    };
}

TArray<FVector> compute_vertex_normals(const TArray<FVector>& verts, const TArray<FIntVector>& faces)
{
    TArray<FVector> smooth_normals;
    smooth_normals.Init(FVector::ZeroVector, verts.Num());

    TArray<int32> count;
    count.Init(0, verts.Num());

    for (const FIntVector& tri : faces)
    {
        FVector a = verts[tri.X];
        FVector b = verts[tri.Y];
        FVector c = verts[tri.Z];
        FVector face_normal = FVector::CrossProduct(b - a, c - a).GetSafeNormal();

        for (int i : { tri.X, tri.Y, tri.Z })
        {
            smooth_normals[i] += face_normal;
            count[i]++;
        }
    }

    for (int i = 0; i < smooth_normals.Num(); ++i)
    {
        if (count[i] > 0)
            smooth_normals[i] = (smooth_normals[i] / count[i]).GetSafeNormal();
    }

    return smooth_normals;
}

int32 get_midpoint(int32 a, int32 b, TMap<uint64, int32>& cache, TArray<FVector>& verts)
{
    uint64 key = (uint64)FMath::Min(a, b) << 32 | FMath::Max(a, b);
    if (cache.Contains(key)) return cache[key];

    FVector midpoint = (verts[a] + verts[b]).GetSafeNormal();
    int32 idx = verts.Add(midpoint);
    cache.Add(key, idx);
    return idx;
}

void subdivide_mesh(TArray<FVector>& verts, TArray<FIntVector>& faces, int level)
{
    for (int l = 0; l < level; ++l)
    {
        TMap<uint64, int32> midpoint_cache;
        TArray<FIntVector> new_faces;

        for (const FIntVector& tri : faces)
        {
            int a = tri.X;
            int b = tri.Y;
            int c = tri.Z;

            int ab = get_midpoint(a, b, midpoint_cache, verts);
            int bc = get_midpoint(b, c, midpoint_cache, verts);
            int ca = get_midpoint(c, a, midpoint_cache, verts);

            new_faces.Add({ a, ab, ca });
            new_faces.Add({ b, bc, ab });
            new_faces.Add({ c, ca, bc });
            new_faces.Add({ ab, bc, ca });
        }

        faces = new_faces;
    }
}

void ASynthwaveCloud::generate_low_poly_cloud()
{
    const float radius = 500.f;
    //const float noise = 15.f;
    const int subdivisions = 2;

    TArray<FVector> verts;
    TArray<FIntVector> tris;
    create_icosahedron(verts, tris);
    subdivide_mesh(verts, tris, subdivisions);

    for (FVector& v : verts)
    {
        FVector dir = v.GetSafeNormal(); // base unit direction
        float noise = FMath::PerlinNoise3D(dir * 1.5f);
        float puff_amount = FMath::Pow(FMath::Clamp(noise, 0.f, 0.1f), 1.5f) * 30.f;

        // Puff direction blending: based on Z height
        float vertical_factor = FMath::Abs(dir.Z) / 2; // 0 at equator, 1 at poles

        // Blend between horizontal (XY only) and full radial (XYZ)
        FVector horiz_dir = FVector(dir.X, dir.Y, 0).GetSafeNormal();
        FVector final_puff_dir = FMath::Lerp(horiz_dir, dir, vertical_factor);

        // Apply puff
        v += final_puff_dir * puff_amount;
    }

    // Scale & distort
    for (FVector& v : verts)
        v = v * radius;// +FMath::VRand() * noise;


    // Build mesh
    TArray<FVector> final_verts;
    TArray<int32> final_tris;
    TArray<FVector> normals;
    TArray<FVector2D> uvs;
    TArray<FLinearColor> colors;
    TArray<FProcMeshTangent> tangents;

    for (const FIntVector& tri : tris)
    {
        final_tris.Add(tri.Z);
        final_tris.Add(tri.Y);
        final_tris.Add(tri.X);

    }


    TArray<FVector> smooth_normals = compute_vertex_normals(verts, tris);

    uvs.Init(FVector2D(0.5f, 0.5f), verts.Num());
    colors.Init(FLinearColor::White, verts.Num());
    tangents.Init(FProcMeshTangent(1, 0, 0), verts.Num());

    cloud_mesh->CreateMeshSection_LinearColor(0, verts, final_tris, smooth_normals, uvs, colors, tangents, false);
    cloud_mesh->SetMaterial(0, CloudMaterial);
}
