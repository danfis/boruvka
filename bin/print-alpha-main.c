#include <fermat/trimesh.h>

int main(int argc, char *argv[])
{
    fer_trimesh_t *obs, *robot;
    fer_vec3_t pos, axis;
    fer_quat_t rot, rot2;
    float x, y, z, w, p, r;

    obs   = ferTriMeshNew();
    robot = ferTriMeshNew();
    ferTriMeshLoad(obs, "alpha-1.5.mesh");
    ferTriMeshLoad(robot, "alpha-1.5.mesh");

    ferTriMeshDumpSVT(obs, stdout, "OBS");

    while(fscanf(stdin, "%f %f %f %f %f %f", &x, &y, &z, &w, &p, &r) == 6){
        ferVec3Set(&pos, x, y, z);

        ferVec3Set(&axis, 1, 0, 0);
        ferQuatSetAngleAxis(&rot, w, &axis);
        ferVec3Set(&axis, 0, 1, 0);
        ferQuatSetAngleAxis(&rot2, p, &axis);
        ferQuatMul(&rot, &rot2);
        ferVec3Set(&axis, 0, 0, 1);
        ferQuatSetAngleAxis(&rot2, r, &axis);
        ferQuatMul(&rot, &rot2);

        ferTriMeshSetRot(robot, &rot);
        ferTriMeshSetPos(robot, &pos);

        ferTriMeshDumpSVT(robot, stdout, "Robot");
    }

    return 0;
}
