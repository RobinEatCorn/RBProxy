#pragma once

#include <asio.hpp>
#include <functional>
#include <utility>
#include <cstddef>

namespace RBProxy {
	class IOInterface {
	public:
		enum OpType { Read, Write };
		enum Status { OK, More, Error, User };

		virtual ~IOInterface() noexcept {};

		virtual OpType op() noexcept = 0;
		virtual void* buf() noexcept = 0;
		virtual size_t len() noexcept = 0;
		virtual Status status() noexcept = 0;
		virtual void operator()() noexcept = 0;

	};

	template<typename Stream>
	class AsioIOBinder : public IOInterface{
	public:
		AsioIOBinder() = default;
		AsioIOBinder(const AsioIOBinder&) = delete;
		AsioIOBinder(AsioIOBinder&&) = default;
		AsioIOBinder& operator=(const AsioIOBinder&) = delete;
		AsioIOBinder& operator=(AsioIOBinder&&) = default;
		virtual ~AsioIOBinder() {};

		AsioIOBinder(Stream&& stream):
			m_stream{std::move(stream)}
		{};

		void set_stream(Stream&& stream){
			m_stream = std::move(stream);
		}

		void process();

	private:
		void transact_and_do(const asio::error_code& ec, std::size_t n_bytes);
		Stream m_stream;
	};

	template<typename Stream>
	void AsioIOBinder<Stream>::transact_and_do(
		const asio::error_code& ec, std::size_t n_bytes)
	{
		if(!ec){
			throw asio::error_code(ec);
		} else {
			(*this)();
			process();
		}
	}

	template<typename Stream>
	void AsioIOBinder<Stream>::process(){
		if(More != status())
		{ return; }

		switch(op()){
		case Read:
			asio::async_read(m_stream, 
				asio::buffer(buf(), len()), 
				std::bind(&AsioIOBinder::transact_and_do, this));
			break;

		case Write:
			asio::async_write(m_stream, 
				asio::buffer(buf(), len()), 
				std::bind(&AsioIOBinder::transact_and_do, this));
			break;

		default:
			assert(0);
			break;
		}
	}
}
