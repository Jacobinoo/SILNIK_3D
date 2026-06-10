/**
 * @file SceneNode.h
 * @brief Wezel grafu sceny - hierarchiczna struktura sceny 3D.
 *
 * Kazdy obiekt na scenie to SceneNode (albo cos co po nim dziedziczy
 * jak Camera, PointLight, PrimitiveNode). Wezly tworza drzewo gdzie
 * transformacja dziecka jest wzgledem rodzica.
 */
#ifndef SCENENODE_H
#define SCENENODE_H

#include "Math3D.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Bazowa klasa hierarchicznego grafu sceny.
 *
 * Trzyma lokalna transformacje (pozycja, rotacja euler, skala) i liste
 * dzieci jako shared_ptr. localMatrix() buduje macierz lokalna z tych
 * komponentow, worldMatrix() schodzi rekurencyjnie do roota mnozac
 * po drodze.
 *
 * Podklasy nadpisuja renderSelf() zeby narysowac swoja geometrie.
 * renderRecursive() jest wywolywana przez silnik na root'cie i sama
 * wywoluje sie po dzieciach.
 */
class SceneNode {
public:
    /** @brief Konstruktor domyslny - nazwa "SceneNode". */
    SceneNode();
    /** @brief Konstruktor z nazwa (do debugowania). */
    explicit SceneNode(const std::string& name);
    /** @brief Wirtualny destruktor (zeby delete dzialal poprawnie na polimorfii). */
    virtual ~SceneNode();

    SceneNode* parent() const;             ///< wskaznik na wezel-rodzica (lub nullptr dla roota)
    const std::string& name() const;       ///< nazwa wezla

    void setPosition(const Vec3& value);   ///< ustawia pozycje lokalna
    void setRotation(const Vec3& value);   ///< ustawia rotacje (euler X, Y, Z w radianach)
    void setScale(const Vec3& value);      ///< ustawia skale (osobne mnozniki per os)

    const Vec3& position() const;
    const Vec3& rotation() const;
    const Vec3& scale() const;

    void translate(const Vec3& delta);     ///< przyrostowa zmiana pozycji
    void rotate(const Vec3& delta);        ///< przyrostowa zmiana rotacji
    void rescale(const Vec3& factor);      ///< mnozy skale (skladowa po skladowej)

    /** @brief Dodaje dziecko. Dziecko trzyma slaby refback przez parentNode. */
    void addChild(const std::shared_ptr<SceneNode>& child);
    /** @brief Lista dzieci tylko do odczytu. */
    const std::vector<std::shared_ptr<SceneNode>>& children() const;

    /**
     * @brief Wlacza/wylacza renderowanie tego wezla.
     *
     * Gdy false to renderSelf NIE jest wywolane, ale dzieci sa nadal
     * rysowane (chociaz w naszej grze nie korzystamy z tego, raczej
     * po prostu chowamy cale poddrzewo). Uzywane przez pule celow -
     * niewidoczne sfery zamiast tworzenia/niszczenia.
     */
    void setVisible(bool v) { nodeVisible = v; }
    bool isVisible() const  { return nodeVisible; }

    /** @brief Macierz lokalna T * Ry * Rx * Rz * S z aktualnych parametrow. */
    Mat4 localMatrix() const;

    /** @brief Macierz swiata - rekurencyjnie skomponowana z przodkow. */
    Mat4 worldMatrix() const;

    /**
     * @brief Rekurencyjne renderowanie wezla i dzieci.
     *
     * parentMatrix to akumulowana macierz transformacji od roota.
     * Po pomnozeniu przez localMatrix() dostajemy world transform tego wezla,
     * potem wolamy renderSelf() i rekurencyjnie po dzieciach.
     */
    void renderRecursive(const Mat4& parentMatrix) const;

protected:
    /**
     * @brief Hook dla podklas zeby narysowac wlasna geometrie.
     *
     * Bazowa implementacja jest pusta - "abstrakcyjny" wezel grupujacy
     * tylko dzieci. Podklasy (CubeNode, PointLight, itd.) nadpisuja.
     */
    virtual void renderSelf(const Mat4& worldMatrix) const;

private:
    std::string nodeName;
    SceneNode* parentNode;
    Vec3 localPosition;
    Vec3 localRotation;
    Vec3 localScale;
    std::vector<std::shared_ptr<SceneNode>> childNodes;
    bool nodeVisible = true;
};

#endif
