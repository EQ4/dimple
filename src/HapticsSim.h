// -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; compile-command:"scons debug=1" -*-

#ifndef _HAPTICS_SIM_H_
#define _HAPTICS_SIM_H_

#include "Simulation.h"
#include "OscObject.h"

#include <CWorld.h>
#include <CCamera.h>
#include <CLight.h>
#include <CMeta3dofPointer.h>

class OscCursorCHAI;

class HapticsSim : public Simulation
{
  public:
    HapticsSim(const char *port);
    virtual ~HapticsSim();

    cWorld *world() { return m_chaiWorld; }

    OscObject *contact_object() { return m_pContactObject; }

    //! Set the grabbed object or ungrab by setting to NULL.
    virtual void set_grabbed(OscObject *pGrabbed);

  protected:
    virtual void initialize();
    virtual void step();

    void findContactObject();

    OscObject *m_pContactObject;
    cVector3d m_lastContactPoint;
    cVector3d m_lastForce;

    //! A step counter
    int m_counter;

    cWorld* m_chaiWorld;            //! the world in which we will create our environment
    OscCursorCHAI* m_cursor;    //! An OscObject representing the 3D cursor.
};

class HapticsPrismFactory : public PrismFactory
{
public:
    HapticsPrismFactory(Simulation *parent) : PrismFactory(parent) {}
    virtual ~HapticsPrismFactory() {}

    virtual HapticsSim* simulation() { return static_cast<HapticsSim*>(m_parent); }

protected:
    bool create(const char *name, float x, float y, float z);
};

class HapticsSphereFactory : public SphereFactory
{
public:
    HapticsSphereFactory(Simulation *parent) : SphereFactory(parent) {}
    virtual ~HapticsSphereFactory() {}

    virtual HapticsSim* simulation() { return static_cast<HapticsSim*>(m_parent); }

protected:
    bool create(const char *name, float x, float y, float z);
};

class HapticsMeshFactory : public MeshFactory
{
public:
    HapticsMeshFactory(Simulation *parent) : MeshFactory(parent) {}
    virtual ~HapticsMeshFactory() {}

    virtual HapticsSim* simulation() { return static_cast<HapticsSim*>(m_parent); }

protected:
    bool create(const char *name, const char *filename,
                float x, float y, float z);
};

class CHAIObject
{
public:
    CHAIObject(cWorld *world);
    virtual ~CHAIObject();

    virtual cGenericObject *object() = 0;

protected:
};

class OscSphereCHAI : public OscSphere, public CHAIObject
{
public:
    OscSphereCHAI(cWorld *world, const char *name, OscBase *parent=NULL);
    virtual ~OscSphereCHAI();

    virtual cShapeSphere *object() { return m_pSphere; }

protected:
    virtual void on_position()
      { object()->setPos(m_position); }
    virtual void on_rotation()
      { object()->setRot(m_rotation); }
    virtual void on_radius();
    virtual void on_color()
      { object()->m_material.m_diffuse.set(m_color.x, m_color.y, m_color.z); }
    virtual void on_visible()
        { object()->setShow(m_visible.m_value, true); }
    virtual void on_friction_static()
        { object()->m_material.setStaticFriction(m_friction_static.m_value); }
    virtual void on_friction_dynamic()
        { object()->m_material.setDynamicFriction(m_friction_dynamic.m_value); }
    virtual void on_grab();

    cShapeSphere *m_pSphere;
};

class OscPrismCHAI : public OscPrism, public CHAIObject
{
public:
    OscPrismCHAI(cWorld *world, const char *name, OscBase *parent=NULL);
    virtual ~OscPrismCHAI();

    virtual cMesh *object() { return m_pPrism; }

protected:
    virtual void on_position()
        { object()->setPos(m_position);
          object()->computeGlobalPositions(); }
    virtual void on_rotation()
        { object()->setRot(m_rotation);
          object()->computeGlobalPositions(); }
    virtual void on_color()
      { object()->m_material.m_diffuse.set(m_color.x, m_color.y, m_color.z); }
    virtual void on_visible()
        { object()->setShow(m_visible.m_value, true); }
    virtual void on_size();
    virtual void on_friction_static()
        { object()->m_material.setStaticFriction(m_friction_static.m_value); }
    virtual void on_friction_dynamic()
        { object()->m_material.setDynamicFriction(m_friction_dynamic.m_value); }
    virtual void on_grab();

    //! Create a cMesh with a prism structure.
    void createPrism(bool openbox=false);

    cMesh *m_pPrism;
};

class OscMeshCHAI : public OscMesh, public CHAIObject
{
public:
    OscMeshCHAI(cWorld *world, const char *name, const char *filename,
                OscBase *parent=NULL);
    virtual ~OscMeshCHAI();

    virtual cMesh *object() { return m_pMesh; }

protected:
    virtual void on_position()
        { object()->setPos(m_position);
          object()->computeGlobalPositions(); }
    virtual void on_rotation()
        { object()->setRot(m_rotation);
          object()->computeGlobalPositions(); }
    virtual void on_color()
      { object()->m_material.m_diffuse.set(m_color.x, m_color.y, m_color.z); }
    virtual void on_friction_static()
        { object()->m_material.setStaticFriction(m_friction_static.m_value); }
    virtual void on_friction_dynamic()
        { object()->m_material.setDynamicFriction(m_friction_dynamic.m_value); }
    virtual void on_visible()
        { object()->setShow(m_visible.m_value, true); }
    virtual void on_size();

    cMesh *m_pMesh;
};

class OscCursorCHAI : public OscSphere, public CHAIObject
{
public:
    OscCursorCHAI(cWorld *world, const char *name, OscBase *parent=NULL);
    virtual ~OscCursorCHAI();

    virtual cMeta3dofPointer *object() { return m_pCursor; }

    bool is_initialized() { return m_bInitialized; }

    int start() { return m_pCursor->start(); }
    int stop()  { return m_pCursor->stop(); }

    void addCursorMassForce();
    void addCursorGrabbedForce(OscObject *pGrabbed);
    void addCursorExtraForce();

protected:
    virtual void on_position()
      { object()->setPos(m_position); }
    virtual void on_rotation()
      { object()->setRot(m_rotation); }
    virtual void on_force();
    virtual void on_radius();
    virtual void on_color()
      { object()->m_colorProxy.set(m_color.x, m_color.y, m_color.z); }
    virtual void on_visible(bool visible)
        { object()->setShow(visible, true); }

    cMeta3dofPointer *m_pCursor;
    cVector3d m_massPos;
    cVector3d m_massVel;
    cVector3d m_lastPosDiff;
    int m_nExtraForceSteps;

    cVector3d m_extraForce;
    bool m_bInitialized;
};

#endif // _HAPTICS_SIM_H_