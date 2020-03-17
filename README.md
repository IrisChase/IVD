# IVD - Interactive Visual Design Language

IVD is a declarative GUI programming language and framework implementation. The goal is to make it so that you can describe *how* a interface should work and leave the rest to the computer, facilitating highly dynamic GUI programming, whilst maintaining maintainability.

# Why?

The decision to create IVD was not reached lightly. I knew it would be a huge undertaking (Although I underestimated how huge), and spent considerable time trying to talk myself out of doing it.

I have experienced developing user interfaces in the classical way (Qt/GTK), the "modern" way (HTML/CSS/Javascript), and played around a bit with QML, which was somewhat inspiring for this project. All of these had their pros and cons, but none of them seemed to get at the heart of the problem.

# A Quick(ish) and Mostly Incomplete Rundown of IVD

## IVD is *Not* CSS++
Before we get started, it's important to acknowledge that although the syntax is superficially similar, the theory of IVD is *very* different from the theory of CSS. Elements in IVD aren't bound to models by default, and they don't inherit attributes from their "parents" (Because they don't really have *true* parents). About the only concepts that carry over from CSS are some attribute names, and the box model for styling. Most everything else is either taken more from traditional GUI toolkits or is novel.

IVD allows for visual elements to be free and not bound to the model, so that your data model isn't corrupted with irrelevant noise which only exists for the presentation, as is always the case with `<DIV>` tags in any reasonably complex webpage.

CSS is for *styling* models, IVD is for *defining* complete user interfaces with [*style*](https://i.redd.it/vq2q5dqh16qy.png).

## Events Kinda Suck, State System to The Rescue
One great problem I think is the low level nature of typical GUI events. Take for example the case of an element needing to style itself differently when hovered in a scroll area. A naive approach involves monitoring "mouse motion in" and "mouse motion out" events, and setting your internal hover state accordingly. This is all fine and good until you have the cursor hovering over an element, and without moving the mouse, the user scrolls, suddenly throwing the hover out of sync.

It's easy enough to fix said issue, but even easier to break again because the solution isn't intuitive. It's fun to watch applications gain and lose this bug as they go through updates. The idea is to leave these tricky edge cases to the GUI framework, instead of fixing and breaking (solved) problems like this all the time while in the middle of trying to fix something completely unrelated.

The problem with individual widgets processing events is that they have no context (At least at the level that your typical GUI events exist, it is certainly possible to have higher level events, but would still require a new framework and IVD as a language has other benefits as well). In IVD, the environment is responsible for determining whether an item is hovered or not (Which was in part inspired by CSS's pseudo-classes):

    #elementName
    {
        color: red;

    state this.IVD-Item-Hovered:
        color: blue;

    state ::.IVD-Mouse-Clicked:
        color: green;

    state ::.IVD-Mouse-Clicked & this.IVD-Item-Hovered:
        color: purple;
    }

Unlike CSS's pseudo-classes, IVD has a very powerful state system and allows boolean comparisons on states:

    state this.aState & (anotherState | !stateeee):

"Overlapping" attributes, where two active states define a different value for a given attribute, override in descending order:

    # //anonymous element
    {
        attr: one;

    state sky-is-blue:
        attr: two;

    state grass-is-green:
        attr: three;        //"three" is chosen by the runtime
    }


Elements can manipulate states, even in other elements:

    #anElement
    {
    state coordinatedState:
        color: blue;
    }

    #
    {
    state aState:
        induce-state: anElement.coordinatedState;

        //Can also toggle, unset, etc
        toggle-state: state;
        unset-state: state;
    }

Event-like behavior is handled by "trigger-states". These are states that are guaranteed by the runtime to last only a single frame. A bit janky conceptually, but it works:

    #
    {
    state this.clicked:
        trigger: IVD-Core-Quit;
    }


Say you have a group of states, and only one can be active at any given time. It could be tabs, or a radio-box selection, etc. IVD can manage the exclusivity for you:

    #
    {
        radio-state: one, two, three;

    state one:
    state two:
    state three:
    }

And then only the last state to be set will be active in that element at any given time.



## Positioning

Widget hierarchies tend to be very ridged. In traditional GUI toolkits, this is because each widget "owns" it's children, and reparenting is an arduous task. HTML isn't much better, where even if you use JavaScript to restructure the DOM, it still has a specific default defined in a very different way from the way that your dynamic structure is defined.

This makes it difficult to create GUIs that are easy to re-arrange.

The two points that make IVD different here is that elements are completely symmetric, and the fact that the visual element's structure isn't tightly bound to the model. The model only enforces it's hierarchy locally; that is, the hierarchy is fixed, but it can be placed anywhere. So you get the power of binding the structure of the layout to a model, but with the flexibility to freely position the elements according to the visual theme, which need not be encoded in the model. ([More on models below](#models)).

IVD's structure being symmetric just means that positioning is always conditional. An element must choose to position itself within another element. It always starts out flat:

    #
    {
    state someCondition:
        position-within: window;

    state someOtherCondition:
        position-within: anotherElement;
    }


## Layouts/Materials

One place where traditional toolkits beat HTML/CSS I think unequivically, is in their layout system. Floats... Are unnecessarily complicated to reason about, and enough tears have been shed over that system that I feel I need not ever speak of it again.

IVD follows a typical nested layout system. Built in layouts include hbox and vbox, for horizontal and vertical rows respectively, and the stack layout for layering.

Given the following example:

    #window
    {
        position-within: Environment; //Give us a window
        layout: hbox;
    }

    #
    {
        position-within: window;
        text: "Aye";
    }

    #
    {
        position-within: window;
        text: "Bye";
    }

    #
    {
        position-within: window;
        text: "Cye";
    }

The following is produced (please excuse the crudity of this model, I didn't have time to build it to scale or paint it):

    [AyeByeCye]

That's great, but then the order is almost arbitrary (It's actually based on the order of element declaration but bear with me). To reserve specific "slots", we have a feature called "named-cells":

    #window
    {
        position-within: Environment; //Give us a window
        layout: hbox;

        named-cells: first, middle, last;
    }

    #
    {
        position-within: window.last;
        text: "Cye";
    }

    #
    {
        position-within: window.first;
        text: "Aye";
    }

    #
    {
        position-within: window.middle;
        text: "Bye";
    }

Which produces the same output:

    [AyeByeCye]

This makes it painless to slip a column in between two elements later in the development cycle, without touching anything that already works. Empty cells are simply ignored, so they're great to declare where a notification or context popout should appear when it feels like it.

Of course, it also makes it trivial to rearrange items:

    #window
    {
        named-cells: first, middle, last;
    
    state a-state:
        named-cells: middle, first, last; //Please use better names tho
    }

Which would produce:

    [ByeAyeCye]

The built-in layouts should cover 95% of use cases simply enough. But for special cases you can define your own.

## Models

A model may define a hierarchy, but elements bound to specific model items don't necessarily have to reflect it directly, and are free to position select leaf items in a root element or separate window, if need be.

The IVD philosophy is that a model shouldn't *ridgedly* define how the presentation should look. Contrast with HTML where absolutely everything is a part of the DOM in a very specific order and hierarchy, even unrelated models must be encoded in an arbitrary order.

There are two types of elements in IVD. "Free elements", which are not bound to a model and there is exactly one instance, and "enumerated elements", which are bound to a model instance, and there can be zero or more of them (one for each instance).

Models can have states:

    # -> modelname
    {
    state model.aState:
        //do something
    }

Models can have trigger slots...


## Equation Solver

IVD allows you to define scalar constraints as equations which are kept up-to-date automatically:

    #element1
    {
        width: otherElement.height * 2;
    }

The trouble with the above, is that you can't set it.

    #element2
    {
    state I-want-to-set-something:
        set: element1 = 200; //error because it doesn't know what to do
    }

Wouldn't it be nice if you could get IVD to figure out what `otherElement.height` needs to be in order for `element1.width` to equal 200?

    #element1
    {
        width: [otherElement.height] * 2; //Declare which value in the expression is weak
    }

    #element2
    {
    state I-want-to-set-something:
        set: element1 = 200; //Works, because now it knows to solve for otherElement.height
    }


What happens in the above is that the expression in `element1.width` is solved for `otherElement.height`, and the result is backpropogated to `otherElement.height` (Which may be defined as a variable or an expression with a weak value, it can be turtles all the way down). Once that is updated, the expression in `element1.width` is reevaluated.

It's important to think of this as more of a suggestion than an absolute order. `otherElement.height` might have a min/max constraint which rounds off the value being propogated, and then THAT value is what is observed when `element1.width` is reevaluated. Nothing is ever left in an inconsistent state.

Everything always obeys the constraints as defined, but you also have the power to update the observed values arbitrarily, giving you the best of both worlds.

## Classes

Classes are very simple. They're really just templates from which attributes are copied:

    .class-name
    {
        color: blue;
    }

    # : class-name
    {
        //color is blue
    }

Shorthand version if you don't need to declare anything in the body of the deriving element:

    # : class-name;

An element can derive from an arbitrary number of classes, and the attributes are overriden in the order that the classes are declared:

    .another-class
    {
        color: yellow;

    state ::.IVD-Mouse-Motion:
        color: green;
    }

    # : another-class, class-name
    {
        //color is blue

    state ::.IVD-Mouse-Motion:
        //color is green
    }

    # : class-name, another-class //Class list order different
    {
        //color is yellow this time

    state ::.IVD-Mouse-Motion:
        //color is still green because it's not in conflict
    }

## Remorial Classes for Code Reuse

And last but *certainly* not least.

Suppose you have the following construct:

    exampLEEE

Everything is fine and perfect and good until... You need to reuse that. You can't simply use a class because a class only helps with a single element, and the above can only work with several elements.

Remorial classes are a way of defining a "composite" element. You define the class as normal, and then attach "remoras" (get it?) which are just a special kind of element to it. Whenever an element derives from this class, a copy of each remora is also spun up as well, facilitating reuse of complex elements.

And the syntax is only minimally different from normal:

    same example but with remoras instead.

Remoras work with models, and common parent deduction and all that good stuff as well. They're just an additional type of template which tag alongside otherwise normal classes.


## And Other Stuff Probably

This is by no means a complete overview of the features developed or in development for IVD. We haven't even mentioned variables or the ability to "declare" coefficients! It is meant to simply give you a taste for the project.

# What's Working?

- The compiler, which produces friendly error messages.
- Basic element styling.
- States and state expressions.
- The layout/material system.
- Text layouts (Although laggy, see [issue](https://github.com/IrisChase/IVD/issues/2)).
- Models.
- Animations of arbitrary scalar attributes.
- The expression solver (Not tested thoroughly enough for my tastes though).

And other things too boring or obvious to list or remember.


# What is blocking alpha?

See [Alpha Release Checklist](https://github.com/IrisChase/IVD/issues/1)

# Contributing

\*sounds of deranged laughter echoing in the distance\*

# Credits

Created and developed by Iris Chase (iris@enesda.com).

# License

IVD is licensed under the terms of the Apache 2.0 license.
