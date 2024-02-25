/*
    https://gist.github.com/dougbinks/65d125e0c11fba81c5e78c546dcfe7af
    // Example use on Windows with links opening in a browser

    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include "Shellapi.h"

    inline void LinkCallback( const char* link_, uint32_t linkLength_ )
    {
        std::string url( link_, linkLength_ );
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    inline void RenderMarkdown( base::ConstCharArray markdown_ )
    {
        MarkdownConfig mdConfig{ LinkCallback, { 4, true, 3, true, 3, false } };
        RenderMarkdown( markdown_.data, markdown_.size, mdConfig );
    }
*/

#include <iostream>
#include <cstdlib>
#include <string>

#include "imgui.h"

#include "imgui_custom/markdown.hpp"

namespace ImGui {
    struct MarkdownConfig {
        static bool linkCallback(const char* link, const uint32_t linkLength) {
            #ifdef _WIN32
                const char cmd[] = "start";
            #elif __APPLE__
                const char cmd[] = "open";
            #else
                const char cmd[] = "xdg-open";
            #endif

            const std::string command { std::string(cmd).append(" ").append(std::string(link, linkLength)) };

            if(std::system(command.c_str()) != 0) {
                std::cerr << "ERROR: ImGui::MarkdownConfig::linkCallback: Failed to open URL." << std::endl;
                return false;
            }

            return true;
        }
        struct HeadingFormat{
            int font;
            bool separator;
        };
        static const int NUMHEADINGS = 3;
        const HeadingFormat headingFormats[NUMHEADINGS] = { 0, true, 0, true, 0, true };
    };

    // External interface

    // Internals
    struct TextRegion;
    struct Line;
    inline void UnderLine(ImColor col_);
    inline void RenderLine(const char* markdown_, Line& line_, TextRegion& textRegion_, const MarkdownConfig& mdConfig_);

    struct TextRegion {
    private:
        float indentX;
        ImFont* pFont;
    public:
        TextRegion() :
            indentX(0.0f)
        {
            pFont = ImGui::GetFont();
        }
        ~TextRegion() {
            ResetIndent();
        }

        // ImGui::TextWrapped will wrap at the starting position
        // so to work around this we render using our own wrapping for the first line
        void RenderTextWrapped(const char* text, const char* text_end, const bool bIndentToHere = false) {
            const float scale = 1.0f;
            float widthLeft = GetContentRegionAvail().x;
            const char* endPrevLine = pFont->CalcWordWrapPositionA(scale, text, text_end, widthLeft);
            ImGui::TextUnformatted(text, endPrevLine);
            if(bIndentToHere) {
                const float indentNeeded = GetContentRegionAvail().x - widthLeft;
                if(indentNeeded) {
                    ImGui::Indent(indentNeeded);
                    indentX += indentNeeded;
                }
            }
            widthLeft = GetContentRegionAvail().x;
            while(endPrevLine < text_end) {
                const char* text = endPrevLine;
                if(*text == ' ') {
                    ++text;
                } // skip a space at start of line
                endPrevLine = pFont->CalcWordWrapPositionA(scale, text, text_end, widthLeft);
                ImGui::TextUnformatted( text, endPrevLine );
            }
        }

        void RenderListTextWrapped(const char* text, const char* text_end) {
            ImGui::Bullet();
            ImGui::SameLine();
            //BeginGroup();
            RenderTextWrapped( text, text_end, true );
            //EndGroup();
        }

        void ResetIndent() {
            if(indentX > 0.0f) {
                ImGui::Unindent( indentX );
            }
            indentX = 0.0f;
        }
    };

    struct Line {						// Text that starts after a new line (or at beginning) and ends with a newline (or at end)
        bool isHeading = false;
        bool isUnorderedListStart = false;
        bool isLeadingSpace = true;		// spaces at start of line
        int leadSpaceCount = 0;
        int headingCount = 0;
        int lineStart = 0;
        int lineEnd   = 0;
        int lastRenderPosition = 0;		// lines may get rendered in multiple pieces
    };

    struct TextBlock {						// subset of line
        int start = 0;
        int stop  = 0;
        int size() const {
            return stop - start;
        }
    };

    struct Link {
        enum LinkState {
            NO_LINK,
            HAS_SQUARE_BRACKET_OPEN,
            HAS_SQUARE_BRACKETS,
            HAS_SQUARE_BRACKETS_ROUND_BRACKET_OPEN,
        };
        LinkState state = NO_LINK;
        TextBlock text;
        TextBlock url;
    };

    inline void UnderLine(const ImColor col_) {
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        min.y = max.y;
        ImGui::GetWindowDrawList()->AddLine( min, max, col_, 1.0f );
    }

    inline void RenderLine(const char* markdown_, Line& line_, TextRegion& textRegion_, const MarkdownConfig& mdConfig_) {
        // indent
        int indentStart = 0;
        if(line_.isUnorderedListStart) {			// ImGui unordered list render always adds one indent
            indentStart = 1; 
        }
        for(int j = indentStart; j < line_.leadSpaceCount / 2; ++j) {	// add indents
            ImGui::Indent();
        }

        // render
        int textStart = line_.lastRenderPosition + 1;
        int textSize = line_.lineEnd - textStart;
        if( line_.isUnorderedListStart ) {			// render unordered list
            const char* text = markdown_ + textStart + 1;
            textRegion_.RenderListTextWrapped( text, text + textSize - 1 );
        }
        else if( line_.isHeading ) {					// render heading
            MarkdownConfig::HeadingFormat fmt;
            if( line_.headingCount > mdConfig_.NUMHEADINGS ) {
                fmt = mdConfig_.headingFormats[ mdConfig_.NUMHEADINGS - 1 ];
            }
            else {
                fmt = mdConfig_.headingFormats[ line_.headingCount - 1 ];
            }

            bool popFontRequired = false;
            if( ImGui::GetIO().Fonts->Fonts.size() > fmt.font ) {
                ImGui::PushFont( ImGui::GetIO().Fonts->Fonts[fmt.font] );
                popFontRequired = true;
            }
            const char* text = markdown_ + textStart + 1;
            ImGui::NewLine();
            textRegion_.RenderTextWrapped( text, text + textSize - 1 );
            if( fmt.separator ) {
                ImGui::Separator();
            }
            ImGui::NewLine();
            if( popFontRequired ) {
                ImGui::PopFont();
            }
        }
        else {									// render a normal paragraph chunk
            const char* text = markdown_ + textStart;
            textRegion_.RenderTextWrapped( text, text + textSize );
        }
            
        // unindent
        for( int j = indentStart; j < line_.leadSpaceCount / 2; ++j ) {	// remove indents
            ImGui::Unindent();
        }
    }
    
    // render markdown
    void RenderMarkdown(const char* markdown_, const uint32_t markdownLength_) {
        const MarkdownConfig mdConfig_{};
        Line line;
        Link link;
        TextRegion textRegion;

        char c = 0;
        for(uint32_t i = 0; i < markdownLength_; i++) {
            c = markdown_[i];			// get the character at index
            if(c == 0) {
                break;
            }			// shouldn't happen but don't go beyond 0.

            // If we're at the beginning of the line, count any spaces
            if(line.isLeadingSpace) {
                if( c == ' ' ) {
                    ++line.leadSpaceCount;
                    continue;
                } else {
                    line.isLeadingSpace = false;
                    line.lastRenderPosition = i - 1;
                    if(( c == '*' ) && ( line.leadSpaceCount >= 2 )) {
                        if(( markdownLength_ > i + 1 ) && ( markdown_[ i + 1 ] == ' ' )) {	// space after '*'
                            line.isUnorderedListStart = true;
                            ++i;
                            ++line.lastRenderPosition;
                        }
                        continue;
                    }
                    else if( c == '#' ) {
                        line.headingCount++;
                        bool bContinueChecking = true;
                        uint32_t j = i;
                        while( ++j < markdownLength_ && bContinueChecking ) {
                            c = markdown_[j];
                            switch( c ) {
                                case '#':
                                    line.headingCount++;
                                    break;
                                case ' ':
                                    line.lastRenderPosition = j - 1;
                                    i = j;
                                    line.isHeading = true;
                                    bContinueChecking = false;
                                    break;
                                default:
                                    line.isHeading = false;
                                    bContinueChecking = false;
                                    break;
                            }
                        }
                        if(line.isHeading) {
                            continue;
                        }
                    }
                }
            }

            // Test to see if we have a link
            switch(link.state) {
                case Link::NO_LINK:
                    if( c == '[' ) {
                        link.state = Link::HAS_SQUARE_BRACKET_OPEN;
                        link.text.start = i + 1;
                    }
                    break;
                case Link::HAS_SQUARE_BRACKET_OPEN:
                    if( c == ']' ) {
                        link.state = Link::HAS_SQUARE_BRACKETS;
                        link.text.stop = i;
                    }
                    break;
                case Link::HAS_SQUARE_BRACKETS:
                    if( c == '(' ) {
                        link.state = Link::HAS_SQUARE_BRACKETS_ROUND_BRACKET_OPEN;
                        link.url.start = i + 1;
                    }
                    break;
                case Link::HAS_SQUARE_BRACKETS_ROUND_BRACKET_OPEN:
                    if( c == ')' ) {	// it's a link, render it.
                        // render previous line content
                        line.lineEnd = link.text.start - 1;
                        RenderLine( markdown_, line, textRegion, mdConfig_ );
                        line.leadSpaceCount = 0;
                        line.isUnorderedListStart = false;	// the following text shouldn't have bullets

                        // render link
                        link.url.stop = i;
                        ImGui::SameLine( 0.0f, 0.0f );
                        ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ ImGuiCol_ButtonHovered ]);
                        ImGui::PushTextWrapPos(-1.0f);
                        const char* text = markdown_ + link.text.start ;
                        ImGui::TextUnformatted( text, text + link.text.size() );
                        ImGui::PopTextWrapPos();
                        ImGui::PopStyleColor();
                        if(ImGui::IsItemHovered()) {
                            // TODO update umgui for ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                            if(ImGui::IsMouseClicked(0)) {
                                mdConfig_.linkCallback(markdown_ + link.url.start, link.url.size());
                            }
                            ImGui::UnderLine( ImGui::GetStyle().Colors[ ImGuiCol_ButtonHovered ] );
                            ImGui::SetTooltip("Open in browser\n%.*s", link.url.size(), markdown_ + link.url.start );
                        } else {
                            ImGui::UnderLine( ImGui::GetStyle().Colors[ ImGuiCol_Button ] );
                        }
                        ImGui::SameLine( 0.0f, 0.0f );
                            
                        // reset the link by reinitialising it
                        link = Link();
                        line.lastRenderPosition = i;
                    }
                    break;
            }

            // handle end of line (render)
            if(c == '\n') {
                // render the line
                line.lineEnd = i;
                RenderLine( markdown_, line, textRegion, mdConfig_ );

                // reset the line
                line = Line();
                line.lineStart = i + 1;
                line.lastRenderPosition = i;

                textRegion.ResetIndent();
                
                // reset the link
                link = Link();
            }
        }

        // render any remaining text if last char wasn't 0
        if(markdownLength_ && line.lineStart < (int)markdownLength_ && markdown_[ line.lineStart ] != 0) {
            line.lineEnd = markdownLength_ - 1;
            RenderLine( markdown_, line, textRegion, mdConfig_ );
        }
    }
}
