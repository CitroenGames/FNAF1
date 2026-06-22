#pragma once

#include <Paingine/Graphics/RenderTarget.hpp>
#include <Paingine/Graphics/View.hpp>

class ScopedView {
public:
    ScopedView(Paingine2D::RenderTarget &target, const Paingine2D::View &view)
        : m_Target(target)
        , m_PreviousView(target.getView()) {
        m_Target.setView(view);
    }

    ScopedView(const ScopedView &) = delete;
    ScopedView &operator=(const ScopedView &) = delete;

    ~ScopedView() {
        m_Target.setView(m_PreviousView);
    }

private:
    Paingine2D::RenderTarget &m_Target;
    Paingine2D::View m_PreviousView;
};
