#ifndef GS_EVENT_H
#define GS_EVENT_H

#include <vector>
#include <map>
#include <tuple>
#include <functional>
#include <algorithm>

namespace gs
{
	/**
	 *	Represents a single event that can be subscribed to and triggered, passing
	 *	a variable number of arguments to subscribers.
	 */
	template<typename... T>
	class event
	{
	public:

		/// Callback Function Type
		typedef std::function<void ( T... )> 	callback;
		/// Structure used to keep track of subscribers
		typedef size_t 				token;

		/**
		 *	Subscribe to this event. The provided callback will be
		 *	invoked when the event is triggered. Use the returned token
		 *	to unsubscribe from the event in the future.
		 * @param c	Callback Function
		 * @return	Token that may be used to unsubscribe from this event
		 */
		token subscribe( callback c )
		{
			m_subscribers[m_token] = c;
			return m_token++;
		}

		/**
		 *	Unsubscribe from this event using a token provided during a previous
		 *	call to subscribe().
		 * @param t	Subscription Token
		 */
		void unsubscribe( token t )
		{
			m_subscribers.erase( t );
		}

		/**
		 *	Trigger this event
		 * @param args	Event Arguments
		 */
		void operator()( T... args )
		{
			for( auto s : m_subscribers )
			{
				s.second( args... );
			}
		}

	private:

		/// Current Subscribers
		std::map<token, callback>	m_subscribers;
		/// Incrementing Token
		token 				m_token = 0;
	};
}

#endif // GS_EVENT_H