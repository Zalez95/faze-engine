#ifndef KEY_EVENT_H
#define KEY_EVENT_H

#include "Event.h"

namespace se::app {

	/**
	 * Class KeyEvent, its an event used for notifying of a key state change
	 * by the InputManager
	 */
	class KeyEvent : public Event<Topic::Key>
	{
	public:		// Nested types
		/** The different state in which a key can be */
		enum class State { Pressed, Released, Repeated };

	private:	// Attributes
		/** The key code of the KeyEvent */
		int mKeyCode;

		/** The state of the key */
		State mState;

	public:		// Functions
		/** Creates a new KeyEvent
		 *
		 * @param	keyCode the key code
		 * @param	state the state of the key */
		KeyEvent(int keyCode, State state) :
			mKeyCode(keyCode), mState(state) {};

		/** @return	the key code of the KeyEvent */
		int getKeyCode() const { return mKeyCode; };

		/** @return	the state of the key */
		State getState() const { return mState; };
	private:
		/** Prints the state to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	s the state to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const State& s
		) {
			switch (s) {
				case State::Pressed:	return os << "State::Pressed";
				case State::Released:	return os << "State::Released";
				case State::Repeated:	return os << "State::Repeated";
				default:				return os;
			}
		};

		/** Appends the current KeyEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current KeyEvent */
		virtual void printTo(std::ostream& os) const
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mKeyCode : " << mKeyCode
				<< ", mState : " << mState << " }";
		};
	};

}

#endif		// KEY_EVENT_H
