#include <fermat/gng-plan.h>
#include <fermat/trimesh.h>

fer_trimesh_t *obs, *robot;

int main(int argc, char *argv[])
{
    fer_vec3_t axis, pos;
    fer_quat_t rot, rot2;

    obs   = ferTriMeshNew();
    robot = ferTriMeshNew();

    ferTriMeshLoad(obs, "alpha-1.5.mesh");
    ferTriMeshLoad(robot, "alpha-1.5.mesh");

    ferVec3Set(&axis, 0, 0, 1);
    ferQuatSetAngleAxis(&rot, -M_PI_4, &axis);

    ferVec3Set(&axis, 0, 1, 0);
    ferQuatSetAngleAxis(&rot2, M_PI_4, &axis);
    ferQuatMul(&rot, &rot2);

    ferVec3Set(&axis, 1, 0, 0);
    ferQuatSetAngleAxis(&rot, M_PI_2, &axis);
    ferTriMeshSetRot(robot, &rot);

    //ferVec3Set(&pos, -15., 0., 10.);
    ferVec3Set(&pos, 10., 15., 3.);
    ferTriMeshSetPos(robot, &pos);

    fprintf(stderr, "%d\n", ferTriMeshCollide(obs, robot));
    ferTriMeshDumpSVT(obs, stdout, "OBS");
    ferTriMeshDumpSVT(robot, stdout, "Robot");

    ferTriMeshDel(obs);
    ferTriMeshDel(robot);

    return 0;
}
