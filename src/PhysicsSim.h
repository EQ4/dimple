// -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; compile-command:"scons debug=1" -*-

#ifndef _PHYSICS_SIM_H_
#define _PHYSICS_SIM_H_

#include "Simulation.h"
#include "OscObject.h"
#include <ode/ode.h>

class PhysicsSim : public Simulation
{
  public:
    PhysicsSim(const char *port);
    virtual ~PhysicsSim();

    dWorldID odeWorld() { return m_odeWorld; }
    dSpaceID odeSpace() { return m_odeSpace; }
    dJointGroupID odeContactGroup() { return m_odeContactGroup; }

    static const int MAX_CONTACTS;

    virtual void on_gravity()
        { dWorldSetGravity(m_odeWorld, m_gravity.x,
                           m_gravity.y, m_gravity.z); }

  protected:
    dWorldID m_odeWorld;
    dSpaceID m_odeSpace;
    dJointGroupID m_odeContactGroup;

    bool m_bGetCollide;
    int m_counter;

    virtual void initialize();
    virtual void step();

    static void ode_errorhandler(int errnum, const char *msg, va_list ap)
        { printf("ODE error %d: %s\n", errnum, msg); }
    static void ode_nearCallback (void *data, dGeomID o1, dGeomID o2);
};

class PhysicsPrismFactory : public PrismFactory
{
public:
    PhysicsPrismFactory(Simulation *parent) : PrismFactory(parent) {}
    virtual ~PhysicsPrismFactory() {}

    virtual PhysicsSim* simulation() { return static_cast<PhysicsSim*>(m_parent); }

protected:
    bool create(const char *name, float x, float y, float z);
};

class PhysicsSphereFactory : public SphereFactory
{
public:
    PhysicsSphereFactory(Simulation *parent) : SphereFactory(parent) {}
    virtual ~PhysicsSphereFactory() {}

    virtual PhysicsSim* simulation() { return static_cast<PhysicsSim*>(m_parent); }

protected:
    bool create(const char *name, float x, float y, float z);
};

class PhysicsHingeFactory : public HingeFactory
{
public:
    PhysicsHingeFactory(Simulation *parent) : HingeFactory(parent) {}
    virtual ~PhysicsHingeFactory() {}

    virtual PhysicsSim* simulation() { return static_cast<PhysicsSim*>(m_parent); }

protected:
    bool create(const char *name, OscObject *object1, OscObject *object2,
                double x, double y, double z, double ax, double ay, double az);
};

class PhysicsFixedFactory : public FixedFactory
{
public:
    PhysicsFixedFactory(Simulation *parent) : FixedFactory(parent) {}
    virtual ~PhysicsFixedFactory() {}

    virtual PhysicsSim* simulation() { return static_cast<PhysicsSim*>(m_parent); }

protected:
    bool create(const char *name, OscObject *object1, OscObject *object2);
};

class PhysicsBallJointFactory : public BallJointFactory
{
public:
    PhysicsBallJointFactory(Simulation *parent) : BallJointFactory(parent) {}
    virtual ~PhysicsBallJointFactory() {}

    virtual PhysicsSim* simulation() { return static_cast<PhysicsSim*>(m_parent); }

protected:
    bool create(const char *name, OscObject *object1, OscObject *object2,
                double x, double y, double z);
};

class ODEObject
{
public:
    ODEObject(dWorldID odeWorld, dSpaceID odeSpace);
    virtual ~ODEObject();

    cVector3d getPosition() { return cVector3d(dBodyGetPosition(m_odeBody)); }
    cMatrix3d getRotation() {
      const dReal *r = dBodyGetRotation(m_odeBody); cMatrix3d m;
      m.set(r[0], r[1], r[2], r[4], r[5], r[6], r[8], r[9], r[10]);
      return m; }

    //! Update ODE dynamics information for this object.
    void update();
    
    //! Remove the association between the body and geom.
    void disconnectBody()
        { dGeomSetBody(m_odeGeom, 0); dBodyDisable(m_odeBody); }
    
    //! Create the association between the body and geom.
    void connectBody()
        { dGeomSetBody(m_odeGeom, m_odeBody); dBodyEnable(m_odeBody); }

protected:
    dBodyID  m_odeBody;
    dGeomID  m_odeGeom;
    dMass	 m_odeMass;
    dWorldID m_odeWorld;
    dSpaceID m_odeSpace;

    friend class ODEConstraint;
};

class ODEConstraint
{
public:
    ODEConstraint(dWorldID odeWorld, dSpaceID odeSpace, OscObject *object1, OscObject *object2);
    virtual ~ODEConstraint();

protected:
    dWorldID m_odeWorld;
    dSpaceID m_odeSpace;
    dJointID m_odeJoint;
    dBodyID  m_odeBody1;
    dBodyID  m_odeBody2;
};

class OscSphereODE : public OscSphere, public ODEObject
{
public:
	OscSphereODE(dWorldID odeWorld, dSpaceID odeSpace, const char *name, OscBase *parent=NULL);
    virtual ~OscSphereODE() {}

protected:
    virtual void on_radius();
    virtual void on_force();
    virtual void on_position()
      { dBodySetPosition(m_odeBody, m_position.x, m_position.y, m_position.z); }
    virtual void on_mass();
};

class OscPrismODE : public OscPrism, public ODEObject
{
public:
	OscPrismODE(dWorldID odeWorld, dSpaceID odeSpace, const char *name, OscBase *parent=NULL);
    virtual ~OscPrismODE() {}

protected:
    virtual void on_size();
    virtual void on_force();
    virtual void on_position()
      { dBodySetPosition(m_odeBody, m_position.x, m_position.y, m_position.z); }
    virtual void on_mass();

    static int push_handler(const char *path, const char *types, lo_arg **argv,
                            int argc, void *data, void *user_data);
};

class OscHingeODE : public OscHinge, public ODEConstraint
{
public:
    OscHingeODE(dWorldID odeWorld, dSpaceID odeSpace,
                const char *name, OscBase *parent, OscObject *object1, OscObject *object2,
                double x, double y, double z, double ax, double ay, double az);

    virtual void simulationCallback();

protected:
    virtual void on_torque() {}
};

class OscFixedODE : public OscFixed, public ODEConstraint
{
public:
    OscFixedODE(dWorldID odeWorld, dSpaceID odeSpace,
                const char *name, OscBase *parent, OscObject *object1, OscObject *object2);

protected:
};

class OscBallJointODE : public OscBallJoint, public ODEConstraint
{
public:
    OscBallJointODE(dWorldID odeWorld, dSpaceID odeSpace,
                    const char *name, OscBase *parent,
                    OscObject *object1, OscObject *object2,
                    double x, double y, double z);
};

#endif // _PHYSICS_SIM_H_
