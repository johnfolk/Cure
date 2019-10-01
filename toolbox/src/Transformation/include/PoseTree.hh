//    $Id: PoseTree.hh,v 1.12 2007/10/26 15:20:36 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2007 John Folkesson
//    

#ifndef CURE_POSETREE_HH
#define CURE_POSETREE_HH

#include "Transformation3D.hh"
#include "Timestamp.hh"
#include "ShortMatrix.hh"
namespace Cure{
  /**
   * This class allows one to define and calculate with and entire
   * tree of transformation links.  So a simple kinetic chain for a
   * robot arm is one example. Another is a robot with several sensors,
   * maybe several on the same pan/tilt unit with others not on it.
   * The tree assumes that there is one root pose (the robot base
   * frame) and then possibly a set link transformations to other
   * frames.  The other frames can then have sets of transformations
   * to other framse and so on.  This forms a tree structure where you
   * must specify the base pose and the link transformations.  One can
   * specify the coviance type, @see Pose3D of each link, the sums of
   * the links/root transformations and the root.  Thus one can say
   * which parts are fixed and known and which are varible or
   * uncertian.  The pose andJacobians of the leafs are important and
   * can be calculated.  The leafs are where the sensors normally will be.
   *
   * @author John Folkesson
   */
  class PoseTree{  
  public:
    /**
     * the link is what connects the frames of interest with
     * independent transformations or link poses.
     */
    class Link{
    public:   
      /** A pointer to the frame lower in the tree, that is its m_ToPose*/
      Cure::Transformation3D *m_FromPose;
      /** The transform of the link, this is the independent part.*/
      Cure::Transformation3D m_LinkPose;
      /** The result of m_ToPose=(*m_FromPose)+m_LinkPose is stored here*/
      Cure::Transformation3D m_ToPose;
      /**This is the list of branches from m_ToPose to other Link objects*/
      Link **m_Links;
      /** this is the length of m_Links*/
      int m_NumberOfLinks;
      /** the CovType of m_LinkPose */
      unsigned short m_LinkType;
      /** The CovType for m_ToPose*/
      unsigned short m_PoseType;
      /** The jacobian between m_ToPose and m_FromPose*/
      Cure::Matrix m_JacToFrom;
      /** The jacobian between m_ToPose and m_LinkPose*/
      Cure::Matrix m_JacToLink;
      /** The argument for adding */
      unsigned short m_AddType;


      /**
       * The copy constructor.
       */
      Link(Link &link){
	m_Links=0;
	m_NumberOfLinks=0;
	(*this)=link;
      }
      /**
       * The copy operator
       */
      void operator=(Link &link);

      /**
       * The constructor.
       * @param posetype the CovType for m_ToPose.
       * @param linktype the CovType for the linkPose
       * @param fromtype the CovType of the frompose
       * @param frompose the pointer to the lower frame's ToPose.
       */
      Link(unsigned short posetype=0,
	   unsigned short linktype=0,  
	   unsigned short fromtype=0,
	   Cure::Transformation3D *frompose=0);

      /**
       * The constructor.
       * @param posetype the CovType for m_ToPose.
       * @param linktype the CovType for the linkPose
       * @param xlink the coordinates of the linkpose
       * @param frompose the pointer to the lower frame's link.
       */
      Link(unsigned short posetype,unsigned short linktype,  
	   double xlink[6], Link *frompose);

      ~Link(){
	for (int i=0;i<m_NumberOfLinks;i++)
	  delete m_Links[i];
	if (m_Links)delete []m_Links;
	m_NumberOfLinks=0;
	m_Links=0;
      }

      /**
       * Set the independent part of the link transformation
       * @param xlink the coordinates of the linkpose
       */
      void setLinkPose(double xlink[6]){m_LinkPose=xlink;}

      /**
       * 
       * Set the link's CovType. This sets the variable coordinates of
       * this link.
       *
       * @param linktype the CovType for the linkPose
       */
      void setLinkType(unsigned short linktype=0);
      
      /**
       * Set the CovType for the sum, m_ToPose.  This determines how
       * the Jacobian will pass this link.  63 is safe as it will not
       * lose any parts of the Jacobian when projected through the
       * m_ToPose coordinates.
       *
       * @param posetype the CovType for m_ToPose.
       */
      void setPoseType(unsigned short posetype);

      /**
       * Set the base pose of this link.
       *
       * @param frompose the pointer to the lower frame's ToPose.
       */
      void setFromPose(Transformation3D *frompose){ m_FromPose=frompose;}
      
      /**
       * Sets the argument for the link calc stage where it
       * must know the Cov type of the m_FromPose.  Also sets the
       * size of the m_JacToFrom.
       *
       * @param fromtype the CovType of the frompose
       */
      void setFromType(unsigned short fromtype);

      /**
       * This creates a new link with the arguments and
       * then adds it to the list of m_Links.
       * @param posetype the CovType for m_ToPose.
       * @param linktype the CovType for the linkPose
       * @param xlink the coordinates of the linkpose
       */
      Link *addlink(unsigned short posetype, unsigned short linktype, 
		    double xlink[6]);
      /**
       * This creates  new links with 0 CovTypes and 0 coordinates. It
       * then adds them to the list of m_Links.
       * @param num the number of links to add. 
       */
      void addLinks(int num);
      /**
       * Will return a link with index n ito the m"_Links array.
       *  If the link does not exist it will make  new links to extend
       * the array.
       */
      Link *makeLink(int n);
      
      /**
       * This calculates the sum of the link's m_FromPose and m_LinkPose
       * putting the result in m_ToPose.  It also calculates the Jacobians.
       * It then calls calc on its m_Links.
       */
      void calc(){
	if (m_FromPose){
	  Matrix j(6,12);
	  m_ToPose.doAplusB(*m_FromPose,m_LinkPose, j,
			    m_PoseType,m_AddType,1);
	  j.Columns=m_JacToFrom.Columns;
	  m_JacToFrom=j;
	  j.offset(m_JacToFrom.Columns,j.Rows,m_JacToLink.Columns);
	  m_JacToLink=j;
	}else {
	  m_JacToLink=1;
	  m_ToPose=m_LinkPose;
	}
	for (int i=0;i<m_NumberOfLinks;i++)
	  m_Links[i]->calc();
      }
      /**
       * This calculates the sum of the link's m_FromPose and m_LinkPose
       * putting the result in m_ToPose.  It also calculates the Jacobians.
       * It then calls calc on its m_Links.
       */
      void calcPoses(){
	if (m_FromPose){
	  Matrix j(6,12);
	  m_ToPose=(*m_FromPose+m_LinkPose);
	}else {
	  m_ToPose=m_LinkPose;
	}
	for (int i=0;i<m_NumberOfLinks;i++)
	  m_Links[i]->calcPoses();
      }
      /**
       * This gets the number of links.
       * @return the count of all link 
       *  
       */
      unsigned short count();
      /**
       * This gets 6 times the number of links.
       * @return the sum of the dimension of all link coordinate vectors.
       *  
       */
      unsigned short dim(){
	unsigned short dimen=6;
	for (int i=0;i<m_NumberOfLinks;i++)
	  dimen+=m_Links[i]->dim();
	return dimen;
      }
      /**
       * This gets the total variable dimensions.
       * @return the sum of the Cov dimension of all link coordinate vectors.
       *  
       */
      unsigned short covDim(){
	unsigned short dim=m_JacToLink.Columns;
	for (int i=0;i<m_NumberOfLinks;i++)
	  dim+=m_Links[i]->covDim();
	return dim;
      }
      /**
       * This is used to get the dimension of the variable parts of the
       * coordinates of a leaf branch.  So only the parts that the 
       * leaf pose depends on.
       *
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       * @return the dimension
       */
      unsigned short leafDim(short *branches);

      /**
       * This returns the entire state vector for the links.
       * @param x the state is returned here, should be length dim()
       * @return dimension of x
       */
      unsigned short  getCoordinates(double *x){
	m_LinkPose.getCoordinates(x);
	unsigned short dim=6;
	for (int i=0;i<m_NumberOfLinks;i++)
	  dim+=m_Links[i]->getCoordinates(x+dim);
	return dim;
      }
      
      /**
       * This returns the link and posetypes for all the links.
       *
       * @param linktypes the linktypes are returned here, should be
       * length dim()/6
       * @param posetypes the posetypes are returned here, should be
       * length dim()/6
       */
      unsigned short getTypes(short *linktypes,short *posetypes);

      /**
       * This sets the link and posetypes for all the links.
       *
       * @param linktypes the linktypes should be
       * length dim()/6
       * @param posetypes the posetypes should be
       * length dim()/6
       */
      unsigned short setTypes(short *linktypes,short *posetypes);

      /**
       * This returns the variable state vector for the links.
       *
       * @param x the variable part of the state is returned here,
       * should be length dim()
       * @return dimension of x
       */
      unsigned short  getCovCoordinates(double *x){
	m_LinkPose.getCovCoordinates(x,m_LinkType);
	unsigned short dim=m_JacToLink.Columns;
	for (int i=0;i<m_NumberOfLinks;i++)
	  dim+=m_Links[i]->getCovCoordinates(x+dim);
	return dim;
      }
      /**
       * This is used to get the variable coordinates of an entire leaf branch.
       *
       * @param x an array of dimension leafDim for the return values.
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       * @return the dimension of x
       */
      unsigned short getLeafCoordinates(double *x,short *branches){
	m_LinkPose.getCovCoordinates(x,m_LinkType);
	unsigned short dim=m_JacToLink.Columns;
	if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
	  dim+=m_Links[branches[0]]->getLeafCoordinates(x+dim,branches+1);
	return dim;
      }
      /**
       * This is used to get the CovType of the leaf pose..
       *
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       * @return the CovType
       */
      unsigned short getLeafPoseType(short *branches){
	if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
	  return m_Links[branches[0]]->getLeafPoseType(branches+1);
	return m_PoseType;
      
      }
      /**
       * This is used to get for example the pose of a sensor
       * at the end of a chain of links.
       *
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       * @return the pose of the leaf.  Null if branches is wrong.
       */
      Cure::Transformation3D *getLeafPose(short *branches){ 
	if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
	  return m_Links[branches[0]]->getLeafPose(branches+1);
	return &m_ToPose;
      }
      /**
       * This is used to get for example the pose of a sensor
       * at the end of a chain of links.  The type is returned as well
       *
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       * @return the pose of the leaf.  Null if branches is wrong.
       */
      void getLeafPose(Transformation3D &pose,unsigned short &type,
		       short *branches){ 
	if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
	  return m_Links[branches[0]]->getLeafPose(pose,type,branches+1);
	pose=m_ToPose;
	type=(m_PoseType);
      }
      /**
       * Gets the link tranfromation of the nth link in the canonical
       * order.  The canonical order is depth first search down the
       * tree where at each node we move to the lower nodes in the
       * order they are listed in the m_Links array.
       *
       * @param n the links are numbered starting from 0 to count()-1
       * @return the pointer to m_LinkPose of link n.
       */
      Cure::Transformation3D * operator()(unsigned short &n){
	unsigned short d=count();
	if (n<d){
	  if (n==0)return &m_LinkPose;
	  n--;
	  for (int i=0;i<m_NumberOfLinks;i++)
	    {
	      Cure::Transformation3D *t=(*m_Links[i])(n);
	      if (t)return t;
	    }
	}
	n-=d;
	return 0;
      }
      /**
       * Gets the link tranformation of the nth link in the canonical
       * order.  The canonical order is depth first search down the
       * tree where at each node we move to the lower nodes in the
       * order they are listed in the m_Links array.
       *
       * @param n the links are numbered starting from 0 to count()-1
       * @param dim the dimension of the CovState is returned here.
       * @param type the CovType of the link is returned here.
       * @return the pointer to m_LinkPose of link n, null if out of bounds.
       */
      Cure::Transformation3D * getLinkPose(unsigned short &n, 
					   unsigned short &dim, 
					   unsigned short &type)
      {
	unsigned short d=count();
	if (n<d){
	  if (n==0){
	    dim=m_JacToLink.Columns;
	    type=m_LinkType;
	    return &m_LinkPose;
	  }
	  n--;
	  for (int i=0;i<m_NumberOfLinks;i++)
	    {
	      Cure::Transformation3D *t=m_Links[i]->getLinkPose(n,dim,type);
	      if (t)return t;
	    }
	}
	n-=d;
	return 0;
      }
      /**
       * Gets the nth link in the canonical
       * order.  The canonical order is depth first search down the
       * tree where at each node we move to the lower nodes in the
       * order they are listed in the m_Links array.
       *
       * @param n the links are numbered starting from 0 to count()-1
       * @return the pointer to  link n, null if out of bounds.
       */
      Link * getLink(unsigned short &n){
	unsigned short d=count();
	if (n<d){
	  if (n==0)  return this;
	  n--;
	  for (int i=0;i<m_NumberOfLinks;i++)
	    {
	      Link *t=m_Links[i]->getLink(n);
	      if (t)return t;
	    }
	}
	n-=d;
	return 0;
      }
      /**
       * This sets the entire state vector for the links.
       * @param x the state should be length dim()
       * @return dimension of x
       */
      unsigned short  setCoordinates(double *x){
	m_LinkPose=x;
	unsigned short dm=6;
	for (int i=0;i<m_NumberOfLinks;i++)
	  dm+=m_Links[i]->setCoordinates(x+dm);
	return dm;
      }
      /**
       * This sets the variable state vector for the links.
       *
       * @param x the variable part of the state
       * should be length dim()
       * @return dimension of x
       */
      unsigned short  setCovCoordinates(double *x){
	m_LinkPose.setCovCoordinates(x,m_LinkType);
	unsigned short dm=m_JacToLink.Columns;
	for (int i=0;i<m_NumberOfLinks;i++)
	  dm+=m_Links[i]->setCovCoordinates(x+dm);
	return dm;
      }
      /**
       * This is used to set the variable coordinates of an entire leaf branch.
       *
       * @param x an array of dimension leafDim with the coordinates.
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       * @return the dimension of x.
       */
      unsigned short setLeafCoordinates(double *x,short *branches){
	m_LinkPose.setCovCoordinates(x,m_LinkType);
	unsigned short dm=m_JacToLink.Columns;
	if (branches[0]<m_NumberOfLinks)
	  dm+=m_Links[branches[0]]->setLeafCoordinates(x+dm,branches+1);
	return dm;
      }
      /**
       * This gets the jacobian of a leaf pose wrt all the variable
       * coordinates that it depends on.  You should call with jac set
       * to the jac of the m_FromPose of this link wrt whatever it
       * depends on.  To add the zero columns for the coordinates one
       * must call addZeros.
       *
       * @param jac returns the jac of the m_ToPose of the leaf wrt all
       * covCoordinates below this link.
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       */
      void getLeafDependentJacobian(Cure::Matrix &jac, short *branches){
	if (jac.Rows!=m_JacToFrom.Columns)
	  jac.reallocateZero(m_JacToFrom.Columns,0);
	Matrix j(m_JacToLink.Rows, jac.Columns);
	j.multiply_(m_JacToFrom,jac);
	jac.reallocate(j.Rows, j.Columns+m_JacToLink.Columns);
	jac.offset(0,j.Columns,j.Rows, m_JacToLink.Columns);
	jac=m_JacToLink;
	jac.offset(0,-j.Columns,j.Rows, j.Columns);
	jac=j;
	jac.offset(0,0,j.Rows, j.Columns+m_JacToLink.Columns);
	if (m_NumberOfLinks==0)return;
	if (branches[0]<m_NumberOfLinks)
	  m_Links[branches[0]]->getLeafDependentJacobian(jac,branches+1);
      }
      /**
       * This adds zero columns to the jacobian of a leaf pose for the
       * variable coordinates that the leaf does not depend on.
       *
       * @param col the column to start adding zeros from.  
       *
       * @param jac called with the jac of the m_ToPose of the leaf wrt all
       * covCoordinates below this link and returns wrt all covVoordinates.
       *
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       */
      void addZeros(unsigned short &col, Cure::Matrix &jac, short *branches){
	unsigned short dim=m_JacToLink.Columns;
	col+=dim;
	dim=0;
	for (unsigned short i=0;i<branches[0];i++)
	  dim+=m_Links[i]->covDim();
	jac.insertColumns(col,dim);
	col+=dim;
	dim=0;
	if (m_NumberOfLinks==0)return;
	if (branches[0]<0)return;
	if (branches[0]<m_NumberOfLinks)
	  m_Links[branches[0]]->addZeros(col,jac,branches+1);
	dim=0;
	for (unsigned short i=branches[0]+1;i<m_NumberOfLinks;i++)
	  dim+=m_Links[i]->covDim();
	jac.insertColumns(col,dim);
	col+=dim;
      }
      /**
       * This gets the jacobian of a leaf pose wrt all the 
       * CovCoordinates.  You should call with jac set to the 
       * jac of the m_FromPose of this link wrt whatever it depends on. 
       *
       * @param jac returns the jac of the m_ToPose of the leaf wrt all
       * covCoordinates.
       * @param branches an array of tree branches that leads to the leaf.
       * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
       * The branches are ignored and the pose returned if the link is a leaf.
       */
      void getLeafJacobian(Cure::Matrix &jac, short *branches){
	if (jac.Rows!=m_JacToFrom.Columns)
	  jac.reallocateZero(m_JacToFrom.Columns,0);
	unsigned short col=jac.Columns;
	getLeafDependentJacobian(jac,branches);
	addZeros(col,jac,branches);
      }      
      void print();
    };


  public:

    Timestamp m_Time;
    /** This is the Pose of the Root  (m_ToPose*/
    Cure::Transformation3D m_RootPose;
    
    /** The CovType for m_RootPose m_ToType*/
    unsigned short m_CovType;
    
  protected:
    /**This is the list of branches from m_ToPose to other Link objects*/
    Link **m_Links;

    /** this is the length of m_Links*/
    int m_NumberOfLinks;

    /** 6 for the root + 6 time the number of links*/
    unsigned short m_Dim;
    /** sum of covDim for all dof*/
    unsigned short m_CovDim;
    
    /**
     * this stores the paths to important 'leafs' a leaf can actually
     * be any node since this will stop when the branch index is -1.
     * So each row holds the indexes into m_Links column one is for
     * the PoseTree->m_Links array, column two is for the next level
     * of links after moving to the first branch, and so on until -1
     * is found.
     */
    ShortMatrix m_Branches;
    /** 
     * 1 for m_Dim ok, 2 for m_CovDim ok, 4 for m_ToLinks ok, and 8 for
     * Jacobians ok.
     */
    unsigned short CalcState;

  public:    
    /**
     * The constructor.
     * @param covtype the CovType for m_RootPose.
     */
    PoseTree(unsigned short covtype=0){
      init();
      m_CovType=covtype;
      reallocate();
    }
    /**
     * The constructor.
     * @param covtype the CovType for m_RootPose.
     * @param x the coordinates of the m_Rootpose
     */
    PoseTree(unsigned short covtype,
	     double x[6]){
      init();
      m_RootPose=x;
      m_CovType=covtype;
      reallocate();
    }

    /**
     * The constructer
     *
     * @param covtype the CovType of the Root
     * rootpose the (x,y,z,theta,phi,psi) of the root
     * @branches each row is the list of branching indexes to get to a
     * leaf along each row with a -1 signaling end.
     *
     * @posetypes each row has the pose types cooresponding to the row
     * of branches, the root does not counts as a pose here.
     *
     * @linktypes each row has the link types cooresponding to the row
     * of branches, the root does not counts as a link here.
     * @linkposees each row has the link coordinates cooresponding to the row
     * of branches
     */
    PoseTree(short covtype, double rootpose[6],
	     ShortMatrix &branches, 
	     ShortMatrix &posetypes,
	     ShortMatrix &linktypes, 
	     Matrix &initlinkposes){
      init();
      m_RootPose=rootpose;
      m_CovType=covtype;
      for (int i=0;i<branches.Rows;i++)
	addBranch(branches.Columns,
		  &branches(i,0), 
		  &posetypes(i,0),
		  &linktypes(i,0), 
		  &initlinkposes(i,0));
      reallocate();
    }
    virtual void init(){
      CalcState=0;
      m_Links=0;
      m_NumberOfLinks=0;
      m_CovType=0;    
      m_Dim=0;
      m_CovDim=0;
    }

    /**
     * The copy constructor.
     */
    PoseTree(PoseTree &tree){
      init();
      (*this)=tree;
    }
    virtual  void clean(){
      m_Branches.reallocate(0);
      for (int i=0;i<m_NumberOfLinks;i++)
	delete m_Links[i];
      if (m_Links)delete []m_Links;
      m_NumberOfLinks=0;
      m_Links=0;
      reallocate();    ;  
    }
    /**
     * The copy operator
     */
    void operator=(PoseTree &tree){
      clean();
      m_Time=tree.m_Time;
      m_Branches=tree.m_Branches;
      m_RootPose=tree.m_RootPose;
      m_NumberOfLinks=tree.m_NumberOfLinks;
      m_Links=new Link*[m_NumberOfLinks];
      m_CovType=tree.m_CovType;
      for (int i=0;i<m_NumberOfLinks;i++){
	m_Links[i]=new Link(*tree.m_Links[i]);
	m_Links[i]->m_FromPose=&m_RootPose;
      }
      reallocate();
    }
    virtual ~PoseTree(){
      clear();
    }
    /**
     * 
     */
    void clear() {
      m_Branches.reallocate(0);
      for (int i=0;i<m_NumberOfLinks;i++)
	delete m_Links[i];
      if (m_Links)delete []m_Links;
      m_NumberOfLinks=0;
      m_Links=0;
    }

    /**
     * This adds a branch to the tree from root to leaf.
     * One specifies all the links from the root to the leaf.
     * They can be given in any order but with multiple specifications of
     * the same link it is the last one that will remain.
     * @param num the number of links.
     * @param branches an array of length num that gives the index of each
     * branch starting at the root->Links and ending with the branch to 
     * the leaf if the branch valued is -1 the brancing is terminated.
     * @param posetypes an array of length num that gives the CovType
     * of the 'to pose' or the pose that is obtained by adding the link
     * pose to the from pose.
     *  
     * @param linktypes an array of length num that gives the CovType
     * of the 'link pose'.  
     * 
     * @param initlinkposes an array of length 6*num that gives the 
     * of the 'link pose' x,y,z,theta,phi,psi.  
     *
     *  root->links[2]->links[3]   
     * ---63--x--0---x--8--
     * -------63-----63-----63
     * 
     *  0 +63=63  +0=63 +8=63
     *
     * num = 2
     * branches  2 3 -1 -1 ... 
     * posetypes 63 63
     * linktypes  0 8
     *
     * Or just the root as a leaf
     * 
     * num = 1
     * branches  -1 -1 -1 ... 
     * posetypes 
     * linktypes 
     *
     * 
     */
    void addBranch(short num,
		   short *branches, 
		   short *posetypes,
		   short *linktypes, 
		   double *initlinkposes);
    /**
     * This adds a branch to the tree from root to leaf.
     * One specifies all the links from the root to the leaf.
     * They can be given in any order but with multiple specifications of
     * the same link it is the last one that will remain.
     * @param num the number of links.
     * @param branches an array of length num that gives the index of each
     * branch starting at the root and ending with the branch to the leaf
     *  -1 terminates this.
     * 
     */
    void addBranch(short num,
		   short *branches);

    /**
     * 
     * Set the root's CovType. This sets the variable coordinates of
     * this root.
     *
     * @param covtype the CovType for the linkPose
     */
    void setCovType(unsigned short covtype=0){
      m_CovType=covtype;
      reallocate();
    }
  protected: 
    /**
     * This creates  new links with 0 CovTypes and 0 coordinates. It
     * then adds them to the list of m_Links.
     * @param num the number of links to add. 
     */
    void addLinks(int num);

    /**
     * Will return a link with index n into the m_Links array.
     *  If the link does not exist it will make  new links to extend
     * the array.
     */
    Link *makeLink(int n);

      
  public:
    /**
     * This calls calc on its m_Links.
     */
    void calc(){
      if (CalcState&8)return;
      for (int i=0;i<m_NumberOfLinks;i++)
	m_Links[i]->calc();
      CalcState=(CalcState|0xC);
    }
    /**
     * This calls calcPoses on its m_Links.
     */
    void calcPoses(){
      if (CalcState&4)return;
      for (int i=0;i<m_NumberOfLinks;i++)
	m_Links[i]->calcPoses();
      CalcState=(CalcState|4);
    }
    /**
     * This gets the number of poses.
     *
     * @return the count of all link poses  plus one for th root pose. 
     *  
     */
    unsigned short count();

    /**
     * This gets 6 times the count.
     * @return the sum of the dimension of all  coordinate vectors.
     *  
     */
    unsigned short dim(){
      if (!(CalcState&1)){
	m_Dim=6;
	for (int i=0;i<m_NumberOfLinks;i++)
	  m_Dim+=m_Links[i]->dim();
	CalcState=(CalcState|1);
      }
      return m_Dim;
    }
    virtual void reallocate(){
      CalcState=0;
    }
    /**
     * This gets the total variable dimensions.
     * @return the sum of the Cov dimension of all link coordinate vectors.
     *  
     */
    unsigned short covDim(){
      if (!(CalcState&2)){
	m_CovDim=calcRows(m_CovType);
	for (int i=0;i<m_NumberOfLinks;i++)
	  m_CovDim+=m_Links[i]->covDim();
	CalcState=(CalcState|2);
      }
      return m_CovDim;
    }
    
    /**
     * This returns the entire state vector for the links.
     * @param x the state is returned here, should be length dim()
     * @return dimension of x
     */
    unsigned short  getCoordinates(double *x){
      calcPoses();
      m_RootPose.getCoordinates(x);
      unsigned short dm=6;
      for (int i=0;i<m_NumberOfLinks;i++)
	dm+=m_Links[i]->getCoordinates(x+dm);
      return dm;
    }
    
    /**
     * This returns the link and posetypes for all the links.
     * So the Root counts as both a link and a pose here.
     * @param linktypes the linktypes are returned here, should be
     * length dim()/6
     * @param posetypes the posetypes are returned here, should be
     * length dim()/6
     */
    unsigned short getTypes(short *linktypes,short *posetypes){
      unsigned short dm=1;
      linktypes[0]=m_CovType;
      posetypes[0]=m_CovType;
      for (int i=0;i<m_NumberOfLinks;i++)
	dm+=m_Links[i]->getTypes(linktypes+dm,posetypes+dm);
      return dm;
    }
 
    /**
     * This sets the link and posetypes for all the links.
     * So the Root counts as both a link and a pose here.
     * @param linktypes the linktypes should be
     * length dim()/6
     * @param posetypes the posetypes should be
     * length dim()/6
     */
    unsigned short setTypes(short *linktypes,short *posetypes){
      m_CovType=linktypes[0];
      unsigned short dm=1;
      for (int i=0;i<m_NumberOfLinks;i++)
	dm+=m_Links[i]->setTypes(linktypes+dm,posetypes+dm);
      return dm;
    } 
    /**
     * This returns the variable state vector for the links.
     *
     * @param x the variable part of the state is returned here,
     * should be length dim()
     * @return dimension of x
     */
    unsigned short  getCovCoordinates(double *x){
      calcPoses();
      m_RootPose.getCovCoordinates(x,m_CovType);
      unsigned short dim=calcRows(m_CovType);
      for (int i=0;i<m_NumberOfLinks;i++)
	dim+=m_Links[i]->getCovCoordinates(x+dim);
      return dim;
    }
    /**
     * This is used to get the CovType of the leaf pose..
     *
     * @param branches an array of tree branches that leads to the leaf.
     * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
     * The branches are ignored and the pose returned if the link is a leaf.
     * @return the CovType
     */
    /*
      unsigned short getLeafPoseType(short *branches){
      if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
      return m_Links[branches[0]]->getLeafPoseType(branches+1);
      return m_CovType;
      }
    */
    /**
     * This is used to get for example the pose of a sensor
     * at the end of a chain of links.
     *
     * @param branches an array of tree branches that leads to the leaf.
     * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
     * The branches are ignored and the pose returned if the link is a leaf.
     * @return the pose of the leaf.  Null if branches is wrong.
     */
    Cure::Transformation3D *getLeafPose(short *branches){ 
      calcPoses();
      if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
	return m_Links[branches[0]]->getLeafPose(branches+1);
      return &m_RootPose;
    }
    /**
     * This is used to get the CovType of the leaf pose..
     *
     * @param branches an array of tree branches that leads to the leaf.
     * so (3,0,1) means follow link[3]->link[0]->link[1] to the leaf.
     * The branches are ignored and the pose returned if the link is a leaf.
     * @return the CovType
     */
    unsigned short getLeafPoseType(unsigned short branchrow){
      if (m_Branches.Columns>1)
	if (m_Branches.Rows>branchrow)
	  if (m_Branches(branchrow,0)>-1)
	    if (m_Branches(branchrow,0)<m_NumberOfLinks)
	      return m_Links[m_Branches(branchrow,0)]
		->getLeafPoseType(&m_Branches(branchrow,1));
      return m_CovType;
    }
    /**
     * 
     * @param pose the tranformation of the root link (from null transform)
     * with its cov type and timestamp is returned in this.
     */
    void getBasePose(Transformation3D &pose,unsigned short &type){
      pose=m_RootPose;
      type=(m_CovType);
    }
    /**
     * 
     * @param pose the tranformation of the leaf (from null transform)
     * @param type the leaf's cov type is returned here
     * with its cov type and timestamp is returned in this.
     */
    void getLeafPose(Transformation3D &pose,
		     unsigned short &type,unsigned short branchrow){
      if (m_Branches.Columns>1)
	if (m_Branches.Rows>branchrow)
	  if (m_Branches(branchrow,0)>-1)
	    if (m_Branches(branchrow,0)<m_NumberOfLinks)
	      return m_Links[m_Branches(branchrow,0)]
		->getLeafPose(pose,type,&m_Branches(branchrow,1));
     
      pose=m_RootPose;
      type=m_CovType;
    }
    

    unsigned short numberOfBranches(){
      return m_Branches.Rows;
    }
    /**
     * This is used to get for example the pose of a sensor
     * at the end of a chain of links.
     *
     * @param branchrow row of m_Branches that leads
     * to the leaf.  so (3,0,1) means follow
     * link[3]->link[0]->link[1] to the leaf.  The branches are
     * ignored and the pose returned if the link is a leaf.  @return
     * the pose of the leaf.  Null if branches is wrong.
     */
    Cure::Transformation3D *getLeafPose(unsigned short branchrow){ 
      if (m_Branches.Columns>1)
	if (m_Branches.Rows>branchrow)
	  if (m_Branches(branchrow,0)>-1)
	    if (m_Branches(branchrow,0)<m_NumberOfLinks){
	      calcPoses();
	      return m_Links[m_Branches(branchrow,0)]
		->getLeafPose(&m_Branches(branchrow,1));
	    }
      return &m_RootPose;
    }
    /**
     * This gets the jacobian of a leaf pose wrt all the 
     * CovCoordinates. 
     * 
     *
     * @param jac returns the jac of the m_ToPose of the leaf wrt all
     * covCoordinates.
     * @param branchrow a row on m_Branches.
     */
    void getLeafJacobian(Cure::Matrix &jac, unsigned short branchrow){
      calc();
      unsigned short col=0;
      getLeafDependentJacobian(jac,branchrow);
      addZeros(col,jac,branchrow);
    }      
    /**
     * Gets the link tranfromation of the nth link in the canonical
     * order.  The canonical order is depth first search down the
     * tree where at each node we move to the lower nodes in the
     * order they are listed in the m_Links array.
     * 
     * @param x the coordinates for the link are returned here
     * @param n the links are numbered starting from 0 to count()-1
     * @return the pointer to m_LinkPose of link n.
     */
    void getLinkPose(double x[6], unsigned short n){
      Cure::Transformation3D *t=(*this)(n);
      if (t)t->getCoordinates(x);
    }
    /**
     * Gets the link tranfromation of the nth link in the canonical
     * order.  The canonical order is depth first search down the
     * tree where at each node we move to the lower nodes in the
     * order they are listed in the m_Links array.
     * 
     * @param trans the pose for the link are returned here
     * @param type  the covVtype of the link
     *@param cdim the covdim of the pose is returned.
     * @param n the links are numbered starting from 0 to count()-1
     * @return 0 if ok else 1
     */
    int getLinkPose(Transformation3D &trans,unsigned short &cdim,
		    unsigned short &type, unsigned short n){
      Cure::Transformation3D *t=getLinkPose(n, cdim,type);
      if (t){
	trans=*t;
	  return 0;
      }
      return 1;
    }
    unsigned short getLinkType(unsigned short n){
       if (n==0)return m_CovType;
      Link *l=getLink(n);
      if (l)return l->m_LinkType;
      return 0;
    }
    /** 
     * Gets the Cov coordinates in x
     * @param x will be set to a column vector of length covDim()
     * @return covDim
     */
    int getX(Cure::Matrix & x){
      x.reallocate(covDim(),1);
      return getCovCoordinates(x.Element);
    }
    
    /**
     * Gets the full state vector 
     * @param x wit be set to a column vector of length dim()
     */
    void getState(Matrix &x){
      x.reallocate(dim(),1);
      PoseTree::getCoordinates(x.Element);
    }

    /**
     * Sets the variable part of the state of the tree.
     * @param x an array of length covDim() to hold the values.
     * 
     */
    void  setX(Matrix &x)
    {  
      if ((x.Rows<covDim())||(x.Columns<1))return;
      setCovCoordinates(x.Element); 
    }
    
    
    /**
     * Sets the full state of the tree.
     * @param x an array of length dim() to hold the values.
     * 
     */
    void setState(const Cure::Matrix & x){
      if ((x.Rows<dim())||(x.Columns<1))return;
      setCoordinates(x.Element);
    }


    unsigned short  setCoordinates(double *x){
      m_RootPose=x;
      unsigned short dim=6;
      for (int i=0;i<m_NumberOfLinks;i++)
	dim+=m_Links[i]->setCoordinates(x+dim);
      CalcState=(CalcState&3);
      return dim;
    }
    /**
     * This sets the variable state vector for the links.
     *
     * @param x the variable part of the state
     * should be length dim()
     * @return dimension of x
     */
    unsigned short  setCovCoordinates(double *x){
      m_RootPose.setCovCoordinates(x,m_CovType);
      unsigned short dim=calcRows(m_CovType);
      for (int i=0;i<m_NumberOfLinks;i++)
	dim+=m_Links[i]->setCovCoordinates(x+dim);
      CalcState=(CalcState&3);
      return dim;
    }
    /**
     * This gets the jacobian of a leaf pose wrt all the variable
     * coordinates that it depends on.  You should call with jac set
     * to the jac of the m_FromPose of this link wrt whatever it
     * depends on.  To add the zero columns for the coordinates one
     * must call addZeros.
     *
     * @param jac returns the jac of the m_ToPose of the leaf wrt all
     * covCoordinates below this link.
     * @param branchrow row of m_Branches
     */
    void getLeafDependentJacobian(Cure::Matrix &jac, unsigned short branchrow)
    {  
      calc();
      int dim=calcRows(m_CovType);	
      jac.reallocateZero(dim);
      jac=1;
      if (m_Branches.Columns>1)
	if (m_Branches.Rows>branchrow)
	    if (m_Branches(branchrow,0)>-1)
	      if (m_Branches(branchrow,0)<m_NumberOfLinks)
		return m_Links[m_Branches(branchrow,0)]
		  ->getLeafDependentJacobian(jac,&m_Branches(branchrow,1));
    }
      /**
       * This adds zero columns to the jacobian of a leaf pose for the
       * variable coordinates that the leaf does not depend on.
       *
       * @param col the column to start adding zeros from.  
       *
       * @param jac called with the jac of the m_ToPose of the leaf wrt all
       * covCoordinates below this link and returns wrt all covVoordinates.
       *
       * @param branchrow  row of m_Branches
       */
      void addZeros(unsigned short &col, Cure::Matrix &jac, 
		    unsigned short branchrow){
	short *branches=0;
	if (m_Branches.Columns>1)
	  if (m_Branches.Rows>branchrow)
	    if (m_Branches(branchrow,0)>-1)
	      if (m_Branches(branchrow,0)<m_NumberOfLinks)
		branches=&m_Branches(branchrow,0);
	unsigned short dm=calcRows(m_CovType);
	col+=dm;
	dm=0;
	if (!branches){
	  jac.insertColumns(col,dim()-col);
	  return;
	}
	for (unsigned short i=0;i<branches[0];i++)
	  dm+=m_Links[i]->covDim();
	jac.insertColumns(col,dm);
	col+=dm;
	dm=0;
	if (m_NumberOfLinks==0)return;
	if (branches[0]<0)return;
	if (branches[0]<m_NumberOfLinks)
	  m_Links[branches[0]]->addZeros(col,jac,branches+1);
	dm=0;
	for (unsigned short i=branches[0]+1;i<m_NumberOfLinks;i++)
	  dm+=m_Links[i]->covDim();
	jac.insertColumns(col,dm);
	col+=dm;
      }
    
    
    /**
     * Sets the root pose.
     * @param x the coordinates of the root pose.
     */  
    void setRootPose(double x[6])
    {
      m_RootPose=x;
      CalcState=(CalcState&3);
    }
      
    /**
     * Sets the link tranfromation of the nth link in the canonical
     * order.  The canonical order is depth first search down the
     * tree where at each node we move to the lower nodes in the
     * order they are listed in the m_Links array.
     * 
     * @param x the coordinates for the link
     * @param n the links are numbered starting from 0 to count()-1
     * @return 0 if ok else 1
     */
    int setLinkPose(double x[6], unsigned short n){
      Cure::Transformation3D *t=(*this)(n);
      CalcState=(CalcState&3);
      if (t)(*t)=x;
      else return 1;
      return 0;
    }

    /**
     * This saves all the info for restoring the tree in two matricies.
     * @param treeinfo stores the short data 
     * @param poseinfo stores the double data
     */
    void save(ShortMatrix &treeinfo, Matrix &poseinfo);      
    /**
     * This resores all the info saved in two matricies saved with save.
     * @param treeinfo stores the short data 
     * @param poseinfo stores the double data
     * @return 0 if succeeds else 1. 
    */
    int restore(ShortMatrix &treeinfo, Matrix &poseinfo, Timestamp &time);
    virtual void print();
  protected:
    /**
     * Gets the link tranfromation of the nth link in the canonical
     * order.  The canonical order is depth first search down the
     * tree where at each node we move to the lower nodes in the
     * order they are listed in the m_Links array.
     *
     * @param n the links are numbered starting from 0 to count()-1
     * @return the pointer to m_LinkPose of link n.
     */
    Cure::Transformation3D * operator()(unsigned short &n){
      unsigned short d=count();
      if (n<d){
	if (n==0)return &m_RootPose;
	n--;
	for (int i=0;i<m_NumberOfLinks;i++)
	  {
	    Cure::Transformation3D *t=(*m_Links[i])(n);
	    if (t)return t;
	  }
      }
      n-=d;
      return 0;
    }

    /**
     * Gets the link tranformation of the nth link in the canonical
     * order.  The canonical order is depth first search down the
     * tree where at each node we move to the lower nodes in the
     * order they are listed in the m_Links array.
     *
     * @param n the links are numbered starting from 0 to count()-1
     * @param dim the dimension of the CovState is returned here.
     * @param type the CovType of the link is returned here.
     * @return the pointer to m_LinkPose of link n, null if out of bounds.
     */
    Cure::Transformation3D * getLinkPose(unsigned short &n, 
					 unsigned short &dim, 
					 unsigned short &type)
    {
      unsigned short d=count();
      if (n<d){
	if (n==0){
	  dim=calcRows(m_CovType);
	  type=m_CovType;
	  return &m_RootPose;
	}
	n--;
	for (int i=0;i<m_NumberOfLinks;i++)
	  {
	    Cure::Transformation3D *t=m_Links[i]->getLinkPose(n,dim,type);
	    if (t)return t;
	  }
      }
      n-=d;
      return 0;
    }

    /**
     * Gets the nth link in the canonical
     * order.  The canonical order is depth first search down the
     * tree where at each node we move to the lower nodes in the
     * order they are listed in the m_Links array.
     *
     * @param n the links are numbered starting from 1 to count()-1
     * @return the pointer to  link n, null if out of bounds.
     */
    Link * getLink(unsigned short &n){
      unsigned short d=count();
      if (n<d){
	if (n==0)  return 0;
	n--;
	for (int i=0;i<m_NumberOfLinks;i++)
	  {
	    Link *t=m_Links[i]->getLink(n);
	    if (t)return t;
	  }
      }
      n-=d;
      return 0;
    }
  };
} // namespace Cure

#endif 
