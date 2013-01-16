#include <exception>


class SkinLayerException: public std::exception
{
private:
	const std::string message;
public:
	SkinLayerException(const std::string message): message(message) {
	}

	virtual ~SkinLayerException() throw() {};

	inline const std::string & getMessage() const {
		return this->message;
	}
};

