/**
* @file
* @author Bernhard Mäder
*/

#ifndef _CLASSVISCOMPONENT_H_
#define _CLASSVISCOMPONENT_H_

class CVisComponent;
class CVisInputPort;			// Forward declaration, #include causes circular reference.

#include "classVisObject.h"
#include "classVisProperty.h"
#include "classVisPort.h"


/**
* @brief The image processing components' base class.
*
* This is the base class for all data processing (be it image data or not) objects in the
* image processing chain.
*/
class CVisComponent : public CVisObject
{
public:
	enum ComponentConsts {	
		COMP_MAX_PROPERTIES = 24,
		COMP_MAX_PORTS		= 8,
		COMP_TYPE_MAX_CHARS = 16		
		};

	enum OperationMode {
		OP_IDLE,
		OP_SERVICE,
		OP_CALIBRATION,
		OP_CLASSIFICATION
		};
		
	/**
	* Instantiates a new vision component using a given name and a given type as identifier.
	* This function is protected so that no CVisComponents can be allocated directly. The function
	* is rather called by a sub-class during construction.
	*/		
protected:
						CVisComponent( const char * strName, const char * strType );
public:						
						
	/**
	* Adds a property to this component. The added property can then be found using getProperty().
	* This function should not be called directly, since it is called by CVisProperty's constructor.
	*/
	bool				AddProperty( CVisProperty * pProperty);

	/**
	* Adds a port to this component. The added port can then be found using getPort().
	* This function should not be called directly, since it is called by CVisPort's constructor.
	*/	
	bool				AddPort( CVisPort * pPort );
	
	/**
	* Sets a component's type. The type is a string identifier that helps to handle components
	* by the user. SetType() is expected to be called by a sub-class in its constructor.
	*
	* The type should identify a component's behaviour and should therefore be corresponding 
	* (or be similar) to it's class name. E.g.: "Threshold", "Label", "Sobel" etc.
	*/
	void				SetType( const char * strType );
	
	/**
	* Returns a component's type.
	*/
	const char *		GetType( );
	
	/**
	* Returns a certain property of this component. The property may be addressed by
	* its name.
	*/
	bool				GetProperty( const char * strName, CVisProperty ** ppProperty );
	
	/**
	* Returns a certain port of this component. The port may be identified by its name.
	*/
	bool				GetPort( const char * strName, CVisPort ** ppPort );
	
	void				SetMainInputPort( CVisInputPort * pPort );
	bool				GetMainInputPort( CVisInputPort ** pPort );	

	void				Prepare();
	
	bool				GetResultImageSize( Uint32 & unWidth, Uint32 & unHeight);
	bool				GetResultImageBPP( Uint32 & unBPP );
	
	/**
	* Connects one of this component's ports to another component's port.
	*/
	bool				Connect( const Char * strLocalPort, CVisComponent * pOtherComp, const Char * strOtherPort );

	/**
	* Disconnects one of this component's ports. Note that only input ports may be disconnected
	* so far.
	*/
	bool				Disconnect( const Char * strLocalPort );

	/**
	* Sets the component's mode of operation, which may be one of service, calibration or 
	* classification. Components usually don't have to act specific on each of those modes, 
	* they're free implement any kind of behaviour. The default mode is operation.
	*/
	void				SetOperationMode( OperationMode mode );
	
	/**
	* Returns whether any of the component's properties have changed since the last call 
	* to ResetPropertiesChanged().
	*
	* Note: The resetting function, ResetPropertiesChanged(), is implemented separately so that
	*		multiple components may listen the the changed event. This is not very good style since
	*		the timing of the reset call musst be made depending on the number and type of listening
	*		components. This should be done using something like an event-listener scheme.
	*/
	bool				HavePropertiesChanged();

	/**
	* Reset the properties changed flag.
	*/
	void				ResetPropertiesChanged();

	/** 
	* Indicates to the component that one of its properties is changing. The next call to 
	* PropertiesChanged() will result in true.
	*/
	void				PropertyChanges();

protected:
	/** The array in which all the component's properties are stored. */
	CVisProperty *		m_aryProperties[COMP_MAX_PROPERTIES];
	
	/** The array in which all the component's ports are stored. */
	CVisPort *			m_aryPorts[COMP_MAX_PORTS];
	
	/** The main input port, if defined. If not defined, the first added input port is used. */
	CVisInputPort *		m_pMainInputPort;
	
	/** The component's type description string. */
	const Char *		m_strType;

	Uint32				m_unResultWidth;
	Uint32				m_unResultHeight;
	Uint32				m_unResultBPP;

	/** The current mode of operation. */
	OperationMode		m_eOperationMode;

private:
	/** A flag to indicate that one of the component's properties has changed since the last call of
		PropertiesChanged(). The flag should not be read directly and thus is private. */
	bool				m_bPropertiesChanged;

};

#endif
