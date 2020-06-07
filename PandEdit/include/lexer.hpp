//  ===== Date Created: 04 May, 2020 ===== 

#if !defined(LEXER_HPP)
#define LEXER_HPP

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "point.hpp"
#include "token.hpp"
#include "line_lex_state.hpp"

class Buffer;

class Lexer
{
public:
	static std::unordered_set<std::string> keywords;
	static std::unordered_set<std::string> primitiveTypes;
	
	Buffer* buffer;
	std::vector<LineLexState> lineStates;
	
public:
	Lexer(Buffer* buffer);
	
	// TODO(fkp): Language of lexing
	void lex(unsigned int startLine, bool lexEntireBuffer);
	void addLine(Point splitPoint);
	void removeLine(Point newPoint);
	std::vector<Token*> getTokens(unsigned int startLine, unsigned int endLine);

private:
	void lexString(Point& point, LineLexState::FinishType& currentLineLastFinishType);
	void lexCharacter(Point& point);
	void lexIncludePath(Point& point);
	void lexLineComment(Point& point);
	void lexBlockComment(Point& point, LineLexState::FinishType& currentLineLastFinishType);
	void lexNumber(Point& point);
	void lexPreprocessorDirective(Point& point);
	bool lexKeyword(const Point& startPoint, const Point& point, const std::string& tokenText);
	void lexIdentifier(const Point& startPoint, const Point& point, const std::string& tokenText);
	bool lexPunctuation(Point& point);

	void doFinalAdjustments();
	std::unordered_map<std::string, std::string> findFunctionsInBuffer();
	
	static bool isIdentifierStartCharacter(char character);
	static bool isIdentifierCharacter(char character);
	static bool isValidHexDigit(char character);
};

#endif
