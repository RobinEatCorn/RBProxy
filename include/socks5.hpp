#include <type_traits>
#include "IOInterface.hpp"
#include <string>
#include <stdint.h>

#ifdef HAVING_INET_H
#	include <arpa/inet.h>
#elif defined(HAVING_WINSOCK2_H)
#	include <Winsock2.h>
#endif


namespace RBProxy { 
	namespace socks5 { 
	
		struct msg_hello_req { 
			alignas(1) uint8_t ver;
			alignas(1) uint8_t nmethods;
			alignas(1) uint8_t methods[255];
		};
		static_assert(0 == offsetof(msg_hello_req, ver));
		static_assert(1 == offsetof(msg_hello_req, nmethods));
		static_assert(2 == offsetof(msg_hello_req, methods));

		struct msg_hello_resp {
			alignas(1) uint8_t ver;
			alignas(1) uint8_t method;
		};
		static_assert(0 == offsetof(msg_hello_resp, ver));
		static_assert(1 == offsetof(msg_hello_resp, method));

		struct msg_negotiation {
			enum CMD : uint8_t { CONNECT = 1, BIND = 2, UDP_ASSOCIATE = 3 };
			enum AType : uint8_t { IPv4 = 1, DomainName = 3, IPv6 = 4 };

			uint8_t ver;
			CMD cmd;
			uint8_t rsv;
			AType atype;
			std::string addr;
			uint16_t port;

			inline uint8_t get_ver() noexcept { return ver; };
			inline CMD get_cmd() noexcept { return cmd; };
			inline uint8_t get_atype() noexcept { return atype; };
			inline std::string get_addr() noexcept { return addr; };
			inline uint16_t get_port() noexcept { return ntohs(port); };
		};

		template<typename Stream>
		class context : public AsioIOBinder<Stream> {
		public:
			context();
			context(const context&) = delete;
			context(context&&) = default;
			context& operator=(const context&) = delete;
			context& operator=(context&&) = default;
			virtual ~context() {};

			inline IOInterface::OpType op() noexcept { return m_op; };
			inline void* buf() noexcept { return m_buf; };
			inline size_t len() noexcept { return m_len; };
			inline IOInterface::Status status() noexcept { return m_status; };
			void operator()() noexcept;

		private:
			msg_hello_req m_hello_req;
			msg_hello_resp m_hello_resp;
			msg_negotiation m_negotiation_req;
			msg_negotiation m_negotiation_resp;

			IOInterface::OpType m_op;
			void* m_buf;
			size_t m_len;
			IOInterface::Status m_status;
			void (context::*m_pc)() noexcept;

			void hello_got_nmethods() noexcept;

		};

		template<typename Stream>
		context<Stream>::context():
			m_op{context<Stream>::Read}, m_buf{&m_hello_req.ver}, 
			m_len{sizeof(msg_hello_req::ver) + sizeof(msg_hello_req::nmethods)},
			m_status{context<Stream>::More}, m_pc{&context<Stream>::hello_got_nmethods}
		{}

		template<typename Stream>
		void context<Stream>::operator()() noexcept {
			assert(nullptr != m_pc);
			(this->*m_pc)();
		}
		
		template<typename Stream>
		void context<Stream>::hello_got_nmethods() noexcept {
		}


		// 所有的 stream 模板共同使用这些枚举类型
		class stream_base {
		public:
			enum handshake_type {
				// 作为客户端进行握手
				client,

				// 作为服务器进行握手
				server
			};

			virtual ~stream_base() {};
		};

		template<typename Stream>
		class stream :
			public stream_base
		{
		public:
			// 下一层的类型
			using next_layer_type = std::remove_reference_t<Stream>;

			// 最底层的类型
			using lowest_layer_type = typename next_layer_type::lowest_layer_type;

			// executor的类型
			using executor_type = typename lowest_layer_type::executor_type;

			template<typename... Args>
			stream(context<Stream>& ctx, Args&&... args):
				m_next_layer(std::forward<Args>(args)...),
				m_context(ctx)
			{}

			virtual ~stream()
			{}

		private:
			next_layer_type m_next_layer;
			context<Stream> m_context;
		};
	}
}
